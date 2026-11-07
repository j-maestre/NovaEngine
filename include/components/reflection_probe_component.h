#pragma once
#include "Core/defines.h"

// Sonda de reflexion: el renderer captura la escena en un cubemap desde la posicion de su transform.
// Los pixeles dentro de la caja (centrada en la sonda, alineada con los ejes del mundo) reflejan
// esa captura en vez del cielo, corregida con proyeccion de caja.
class ReflectionProbeComponent {

public:
	// Lado de la caja en cada eje, en metros. Debe coincidir con las paredes de la habitacion.
	Vec3 size = { 4.0f, 3.0f, 4.0f };
	// Metros desde el borde hacia dentro en los que la sonda entra poco a poco
	float blend = 0.3f;
	float intensity = 1.0f;
};
