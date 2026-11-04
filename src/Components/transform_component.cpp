
#include "components/transform_component.h"


TransformComponent::TransformComponent() : m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {
	m_is_calculated = false;
}


TransformComponent::TransformComponent(const Vec3& postion) : m_position(postion), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {
	m_is_calculated = false;
}

TransformComponent::TransformComponent(const Vec3& postion, const Vec3& scale) : m_position(postion), m_scale(scale), m_rotation(0.0f, 0.0f, 0.0f){
	m_is_calculated = false;
}

TransformComponent::TransformComponent(const Vec3& postion, const Vec3& scale, const Vec3& rotation) : m_position(postion), m_scale(scale), m_rotation(rotation){
	m_is_calculated = false;
}

TransformComponent::TransformComponent(const TransformComponent& other)
	: m_position(other.m_position), m_rotation(other.m_rotation), m_scale(other.m_scale), m_parent(other.m_parent) {
	m_is_calculated = false;
}

TransformComponent::TransformComponent(TransformComponent&& other)
	: m_position(other.m_position), m_rotation(other.m_rotation), m_scale(other.m_scale), m_parent(other.m_parent) {
	m_is_calculated = false;
}

TransformComponent::~TransformComponent(){

}

void TransformComponent::update(){

	// Primero el padre: si se ha recalculado, su version cambia, aunque ya este marcado como calculado
	unsigned int parent_version = 0;
	if (m_parent) {
		m_parent->update();
		parent_version = m_parent->m_version;
	}

	if (m_is_calculated && parent_version == m_parent_version) return;

	Mat4 scale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	Mat4 rotationX = DirectX::XMMatrixRotationX(m_rotation.x);
	Mat4 rotationY = DirectX::XMMatrixRotationY(m_rotation.y);
	Mat4 rotationZ = DirectX::XMMatrixRotationZ(m_rotation.z);
	Mat4 rotation = rotationZ * rotationY * rotationX;

	Mat4 translation = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	
	// Model construction
	//m_transform = translation * rotation * scale;
	m_transform = scale * rotation * translation;

	// Parenting
	if (m_parent) {
		m_transform *= m_parent->m_transform;
	}

	// Raw transform
	//Mat4 raw = DirectX::XMMatrixTranspose(m_transform);
	DirectX::XMStoreFloat4x4(&m_transform_float, m_transform);
	m_transform_raw = reinterpret_cast<float*>(&m_transform_float);

	// Inverse model
	m_inverse_transform = DirectX::XMMatrixInverse(nullptr, m_transform);

	m_is_calculated = true;
	m_parent_version = parent_version;
	m_version++;
}

void TransformComponent::force_update(){
	m_is_calculated = false;
}

void TransformComponent::rotateX(float angle){
	m_rotation.x += angle;
	m_is_calculated = false;
}

void TransformComponent::rotateY(float angle){
	m_rotation.y += angle;
	m_is_calculated = false;
}

void TransformComponent::rotateZ(float angle){
	m_rotation.z += angle;
	m_is_calculated = false;
}

void TransformComponent::rotateXYZ(float x, float y, float z){

	m_rotation.x += x;
	m_rotation.y += y;
	m_rotation.z += z;
	m_is_calculated = false;
}

void TransformComponent::rotateXYZ(const Vec3& rotation){
	m_rotation.x += rotation.x;
	m_rotation.y += rotation.y;
	m_rotation.z += rotation.z;
	m_is_calculated = false;
}

void TransformComponent::rotateXYZ(const float rotation[3]){
	m_rotation.x += rotation[0];
	m_rotation.y += rotation[1];
	m_rotation.z += rotation[2];
	m_is_calculated = false;
}

void TransformComponent::set_position(const Vec3& position){

	m_position = position;
	m_is_calculated = false;
}

void TransformComponent::set_position(const float position[3]){
	m_position.x = position[0];
	m_position.y = position[1];
	m_position.z = position[2];
	m_is_calculated = false;
}

void TransformComponent::set_scale(const Vec3& scale){

	m_scale = scale;
	m_is_calculated = false;
}

void TransformComponent::set_scale(const float scale[3]){
	m_scale.x = scale[0];
	m_scale.y = scale[1];
	m_scale.z = scale[2];
	m_is_calculated = false;
}

void TransformComponent::set_rotation(const Vec3& rotation){
	m_rotation = rotation;
	m_is_calculated = false;
}

void TransformComponent::set_rotation(const float rotation[3]){
	m_rotation.x = rotation[0];
	m_rotation.y = rotation[1];
	m_rotation.z = rotation[2];
	m_is_calculated = false;
}

void TransformComponent::set_parent(TransformComponent* parent){
	m_parent = parent;
	m_is_calculated = false;
}

bool TransformComponent::is_descendant_of(const TransformComponent* other) const{

	for (const TransformComponent* p = m_parent; p != nullptr; p = p->m_parent) {
		if (p == other) return true;
	}
	return false;
}

bool TransformComponent::set_parent_keep_world(TransformComponent* parent){

	if (parent == this || (parent && parent->is_descendant_of(this))) return false;

	const Mat4 world = get_transform();
	m_parent = parent;
	return set_world_matrix(world);
}

bool TransformComponent::set_world_matrix(const Mat4& world){

	Mat4 local = world;
	if (m_parent) {
		local = world * DirectX::XMMatrixInverse(nullptr, m_parent->get_transform());
	}

	FVector scale, rotation, translation;
	if (!DirectX::XMMatrixDecompose(&scale, &rotation, &translation, local)) {
		m_is_calculated = false;
		return false;
	}

	DirectX::XMStoreFloat3(&m_position, translation);
	DirectX::XMStoreFloat3(&m_scale, scale);

	// update() compone la rotacion como Rz * Ry * Rx: esta extraccion es su inversa
	DirectX::XMFLOAT4X4 r;
	DirectX::XMStoreFloat4x4(&r, DirectX::XMMatrixRotationQuaternion(rotation));

	const float sin_y = r._31 > 1.0f ? 1.0f : (r._31 < -1.0f ? -1.0f : r._31);
	m_rotation.y = asinf(sin_y);
	if (fabsf(sin_y) < 0.9999f) {
		m_rotation.x = atan2f(-r._32, r._33);
		m_rotation.z = atan2f(-r._21, r._11);
	}
	else {
		// Gimbal lock: X y Z giran sobre el mismo eje
		m_rotation.x = 0.0f;
		m_rotation.z = atan2f(r._12, r._22);
	}

	m_is_calculated = false;
	return true;
}

Vec3& TransformComponent::get_position(){

	return m_position;
}

Vec3& TransformComponent::get_rotation(){

	return m_rotation;
}

Vec3& TransformComponent::get_scale()
{
	return m_scale;
}

const Vec3 TransformComponent::get_forward_vector()
{
	return Vec3();
}

const Vec3 TransformComponent::get_right_vector()
{
	return Vec3();
}

const Vec3 TransformComponent::get_up_vector()
{
	return Vec3();
}

const Mat4 TransformComponent::get_transform() {
	update();
	return m_transform;
}

const Mat4 TransformComponent::get_inverse_transform() {
	update();
	return m_inverse_transform;
}

float* TransformComponent::get_transform_raw(){

	update();
	return m_transform_raw;
}
