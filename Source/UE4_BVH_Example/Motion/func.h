#pragma once

#include "ml.h"

namespace ml {

	double DiffPosture(const Posture& p1, const Posture& p2);
	double DiffPoseLocalPos(const Posture& p1, const Posture& p2);
	double DiffPoseLocalPos(const Motion& m1, int f1, const Motion& m2, int f2);
	double DiffPoseGlobalPos(const Posture& p1, const Posture& p2);
	Motion& stitch(Motion m1, Motion m2, int warp_width = 20);

	std::pair<cml::vector3d, std::vector<cml::vector3d>> difference(const Posture &p2, const Posture &p1);
	void add_difference(Posture &p, std::pair<cml::vector3d, std::vector<cml::vector3d>> &diff, double ratio);

	std::pair<cml::vector3d, cml::vector3d> difference_root(const Posture &p2, const Posture &p1);
	void add_difference_root_direction_height(Posture &p, std::pair<cml::vector3d, cml::vector3d> &diff, double ratio);

	double scalarTransitionFunc(const double t, const double range);

	void warp( Motion * before_m, Motion * after_m );
	void warp_root_direction_height( Motion * before_m, Motion * after_m, int before_warp_width, int after_warp_width );
};

