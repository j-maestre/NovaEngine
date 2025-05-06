#pragma once
#include "memory"
#include "Core/defines.h"
#include "components/Lights/Light.h"



class PointLight : public Light{

public:
	PointLight();
	PointLight(float aspect_ratio);
	PointLight(const PointLight&);
	PointLight(PointLight&&);
	~PointLight();

	void upload_data() override;

private:


};
