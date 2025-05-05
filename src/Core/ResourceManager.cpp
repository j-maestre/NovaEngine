#include "Core/ResourceManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "Core/engine.h"


ResourceManager::ResourceManager(){
	
}

void ResourceManager::set_engine(Engine* e){
	m_engine = e;
}

ResourceManager::ResourceManager(const ResourceManager&)
{
}

ResourceManager::ResourceManager(ResourceManager&&)
{
}

ResourceManager::~ResourceManager(){
	for (auto t : m_textures) {
		t.second.get_data()->texture_view->Release();
		t.second.get_data()->texture->Release();
	}

	for (auto model : m_models) {
		for (Mesh m : model.second.meshes) {
			m.index_buffer->Release();
			m.buffer->Release();
		}
	}
}

Texture* ResourceManager::load_texture(std::string path){

	unsigned int hash = std::hash<std::string>{}(path);
	
	if (m_textures.contains(hash)) {
		return &(m_textures.find(hash)->second);
	}

	Texture texture_class(hash);
	ImageData* data = texture_class.get_data();

	//stbi_set_flip_vertically_on_load_thread(1);
	//stbi_set_flip_vertically_on_load(true);
	unsigned char* pixels = stbi_load(path.c_str(), &data->width, &data->height, &data->channels,4);

	if (!pixels) {
		printf("*** Error loading image %s ***\n", path.c_str());
		return nullptr;
	}

	
	D3D11_TEXTURE2D_DESC texture_desc{};
	texture_desc.Width = data->width;
	texture_desc.Height = data->height;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags= D3D11_BIND_SHADER_RESOURCE;
	
	D3D11_SUBRESOURCE_DATA init_data{};
	init_data.pSysMem = pixels;
	init_data.SysMemPitch = data->width * 4;
	
	
	
	HRESULT hr = m_engine->get_engine_props()->deviceInterface->CreateTexture2D(&texture_desc, &init_data, &data->texture);
	if (FAILED(hr)) {
		stbi_image_free(pixels);
		return nullptr;
	}
	
	data->texture_view = nullptr;
	hr = m_engine->get_engine_props()->deviceInterface->CreateShaderResourceView(data->texture, nullptr, &data->texture_view);
	//texture->Release();
	if (FAILED(hr)) {
		stbi_image_free(pixels);
		return nullptr;
	}

	stbi_image_free(pixels);
	
	m_textures.insert(std::pair(hash, texture_class));
	return &(m_textures.find(texture_class.get_id())->second);
	
}

Texture* ResourceManager::get_texture(unsigned int id){

	return &(m_textures.find(id)->second);
}

Model* ResourceManager::load_mesh(std::string path){

	unsigned int hash = std::hash<std::string>{}(path);
	
	if (m_models.contains(hash)) {
		return &(m_models.find(hash)->second);
	}

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path.c_str(),
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices);

	const char* ret = importer.GetErrorString();
	assert(scene && "Error loading mesh");

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf("Error loading mesh\n %s\n", importer.GetErrorString());
		return nullptr;
	}

	std::string full_path = path;
	char c;
	bool interrupt = false;
	do {
		c = full_path.back();
		if (c != '/') {
			full_path.pop_back();
		}
		else { interrupt = true; }
	} while (interrupt == false);


	aiNode* rootNode = scene->mRootNode;

	Model model;
	ProcessNode(&model, rootNode, scene, full_path);

	// Create buffer

	for (Mesh& m : model.meshes) {

		// Vertex buffer
		D3D11_BUFFER_DESC buffer_desc{};
		ZeroMemory(&buffer_desc, sizeof(buffer_desc));
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;					// Write acces by CPU and GPU
		buffer_desc.ByteWidth = m.num_vertices * sizeof(Vertex);
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// Using as Vertex buffer
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// allow CPU to write in the buffer
		Engine::get_instance()->get_engine_props()->deviceInterface->CreateBuffer(&buffer_desc, NULL, &m.buffer);

		D3D11_MAPPED_SUBRESOURCE ms_mesh;



		// Index buffer
		D3D11_BUFFER_DESC index_buffer_desc{};
		ZeroMemory(&index_buffer_desc, sizeof(index_buffer_desc));
		index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		index_buffer_desc.ByteWidth = m.num_indices * sizeof(unsigned int);
		index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		index_buffer_desc.CPUAccessFlags = 0;
		index_buffer_desc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA init_data = {};
		init_data.pSysMem = m.indices.data();

		m.index_buffer = nullptr;
		HRESULT hr = Engine::get_instance()->get_engine_props()->deviceInterface->CreateBuffer(&index_buffer_desc, &init_data, &m.index_buffer);
		if (FAILED(hr)) {
			assert("Buffer creation failed");
		}
		Engine::get_instance()->get_engine_props()->inmediateDeviceContext->Map(m.buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms_mesh);
		memcpy(ms_mesh.pData, m.vertices.data(), sizeof(Vertex) * m.num_vertices);
		Engine::get_instance()->get_engine_props()->inmediateDeviceContext->Unmap(m.buffer, NULL);
	}

	m_models.insert(std::pair(hash, model));
	return &(m_models.find(hash)->second);


}

void ResourceManager::ProcessNode(Model* model, aiNode* node, const aiScene* scene, std::string absolute_path){

	// Process all node's meshes
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {

		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh mesh_tmp;
		ProcessMesh(&mesh_tmp, mesh, scene, absolute_path);
		model->meshes.push_back(std::move(mesh_tmp));
		// TODO: Peta aqui, y es porque se esta llamano al destructor de Mesh y al destructor de la sce_texture, hay que moverlo y que no se llame al destructor de texture
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ProcessNode(std::move(model), node->mChildren[i], scene, absolute_path);
	}


}

void ResourceManager::ProcessMesh(Mesh* mesh, aiMesh* assimp_mesh, const aiScene* scene, std::string absolute_path){



	mesh->num_vertices= assimp_mesh->mNumVertices;

	// Position, Normals & UVs
	for (unsigned int i = 0; i < assimp_mesh->mNumVertices; i++) {
		Vertex vertex{
			.pos_x = assimp_mesh->mVertices[i].x,
			.pos_y = assimp_mesh->mVertices[i].y,
			.pos_z = assimp_mesh->mVertices[i].z,
			.nrm_x = assimp_mesh->mNormals[i].x,
			.nrm_y = assimp_mesh->mNormals[i].y,
			.nrm_z = assimp_mesh->mNormals[i].z,
			.uv_x = assimp_mesh->mTextureCoords[0][i].x,
			.uv_y = assimp_mesh->mTextureCoords[0][i].y,
		};
		mesh->vertices.push_back(vertex);
	}

	// Indices
	for (unsigned int i = 0; i < assimp_mesh->mNumFaces; i++) {
		const aiFace& face = assimp_mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			mesh->indices.push_back(face.mIndices[j]);
		}
	}

	mesh->num_indices = mesh->indices.size();

	mesh->material = *(m_engine->get_default_material());

	//mesh->material = nullptr;
	
	// MaterialIndex is the index of the own material in the global material array
	if (assimp_mesh->mMaterialIndex >= 0) {
		
		//mesh->material = new MaterialComponent;

		//Tyro::Texture tex;

		const aiMaterial* material = scene->mMaterials[assimp_mesh->mMaterialIndex];
		//tex.m_textureName = material->GetName().C_Str();

		// Get uniform color
		aiColor4D color;
		if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
			//mesh_processed->m_material.set_color(color.r, color.g, color.b, color.a);
			//model->m_material.set_color(1.0f, 0.0f, 0.0f, 1.0f);
		}

		// Get diffuse color
		aiString texturePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {

			Texture diffuse_tex;
			//diffuse_tex.m_textureName = "diffuse";

			std::string tmp(absolute_path + texturePath.C_Str());
			
			Texture* t = load_texture(tmp);
			mesh->material.set_texture_albedo(t);
			

		}



		texturePath.Clear();

		/*
		if (material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == AI_SUCCESS) {
			Tyro::Texture diffuse_tex;
			diffuse_tex.m_textureName = "unknow";

		}
		

		// Get normal texture
		if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {

			Tyro::Texture diffuse_tex;
			diffuse_tex.m_textureName = "normals";

			std::string tmp(texturePath.C_Str());
			NormalizePath(tmp);
			RemoveAbsolute(tmp);
			std::string real_path = tmp + std::string(".gnf");

			// load gnf image
			std::string complete_path(m_fbx_full_path + real_path);
			unsigned int id = LoadTexture(complete_path.c_str());
			mesh_processed->m_material.add_texture("normals", id);
		}

		// Get metallic texture
		if (material->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS) {

			Tyro::Texture diffuse_tex;
			diffuse_tex.m_textureName = "metal";

			std::string tmp(texturePath.C_Str());
			NormalizePath(tmp);
			RemoveAbsolute(tmp);
			std::string real_path = tmp + std::string(".gnf");

			// load gnf image
			std::string complete_path(m_fbx_full_path + real_path);
			unsigned int id = LoadTexture(complete_path.c_str());
			mesh_processed->m_material.add_texture("metal", id);
		}

		// Get roughness texture
		if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath) == AI_SUCCESS) {

			Tyro::Texture diffuse_tex;
			diffuse_tex.m_textureName = "roughness";

			std::string tmp(texturePath.C_Str());
			NormalizePath(tmp);
			RemoveAbsolute(tmp);
			std::string real_path = tmp + std::string(".gnf");

			// load gnf image
			std::string complete_path(m_fbx_full_path + real_path);
			unsigned int id = LoadTexture(complete_path.c_str());
			mesh_processed->m_material.add_texture("roughness", id);
		}

		// Get ambient oclusion texture
		if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS) {

			Tyro::Texture diffuse_tex;
			diffuse_tex.m_textureName = "ambient_oclusion";

			std::string tmp(texturePath.C_Str());
			NormalizePath(tmp);
			RemoveAbsolute(tmp);
			std::string real_path = tmp + std::string(".gnf");

			// load gnf image
			std::string complete_path(m_fbx_full_path + real_path);
			unsigned int id = LoadTexture(complete_path.c_str());
			mesh_processed->m_material.add_texture("ambient_oclusion", id);
		}

		*/

	}

}
