#pragma once
#include "memory"
#include "Core/defines.h"
#include "components/Lights/Light.h"


class SpotLight : public Light {

public:
	SpotLight();
	SpotLight(float aspect_ratio);
	SpotLight(const SpotLight&);
	SpotLight(SpotLight&&);
	~SpotLight();


	void upload_data() override;


private:


};

