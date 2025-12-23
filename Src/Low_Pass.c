#include <Low_Pass.h>
float a[FORDER +1] =
{
	1.000000000000000, -4.438467815579958, 8.854915480723749, -9.975142272550261,
	6.658387733940518, -2.487622308316360, 0.407475776275259
};

float b[FORDER +1] =
{
	0.040417532280911, -0.127344926885810, 0.231744360220144, -0.270532271549055,
	0.231744360220144, -0.127344926885810, 0.040417532280911
};

float filter_compute(float xin)
{
	static float x[FORDER + 1] = {0};
	static float y[FORDER + 1] = {0};

	/*Update samples*/
	for (int k = FORDER; k > 0; k--){
		y[k] = y[k-1];
		x[k] = x[k-1];
	}
	x[0] = xin;

	/*Compute filter output*/
	y[0] = b[0] * x[0];
	for(int i = 1; i <= FORDER; i++){
		y[0] += b[i] * x[i] - a[i] * y[i];
	}
	return y[0];
}



