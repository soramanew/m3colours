#pragma once

namespace m3colours::contrast {

double ratioOfYs(double y1, double y2);
double ratioOfTones(double toneA, double toneB);
double lighter(double tone, double ratio);
double darker(double tone, double ratio);
double lighterUnsafe(double tone, double ratio);
double darkerUnsafe(double tone, double ratio);

} // namespace m3colours::contrast
