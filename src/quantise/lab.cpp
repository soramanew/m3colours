#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

Lab::Lab(double l, double a, double b)
    : l(l)
    , a(a)
    , b(b) {}

Lab::Lab(Argb argb) {
    const auto redL = detail::linearise(argb.red());
    const auto greenL = detail::linearise(argb.green());
    const auto blueL = detail::linearise(argb.blue());

    const auto x = 0.41233895 * redL + 0.35762064 * greenL + 0.18051042 * blueL;
    const auto y = 0.2126 * redL + 0.7152 * greenL + 0.0722 * blueL;
    const auto z = 0.01932141 * redL + 0.11916382 * greenL + 0.95034478 * blueL;

    const auto yNormalised = y / k_whitePointD65[1];
    const auto e = 216.0 / 24389.0;
    const auto kappa = 24389.0 / 27.0;
    const auto fy = yNormalised > e ? std::pow(yNormalised, 1.0 / 3.0) : (kappa * yNormalised + 16) / 116;
    const auto xNormalised = x / k_whitePointD65[0];
    const auto fx = xNormalised > e ? std::pow(xNormalised, 1.0 / 3.0) : (kappa * xNormalised + 16) / 116;
    const auto zNormalised = z / k_whitePointD65[2];
    const auto fz = zNormalised > e ? std::pow(zNormalised, 1.0 / 3.0) : (kappa * zNormalised + 16) / 116;

    l = 116.0 * fy - 16;
    a = 500.0 * (fx - fy);
    b = 200.0 * (fy - fz);
}

Argb Lab::toArgb() const {
    const auto e = 216.0 / 24389.0;
    const auto kappa = 24389.0 / 27.0;
    const auto ke = 8.0;

    const auto fy = (l + 16.0) / 116.0;
    const auto fx = (a / 500.0) + fy;
    const auto fz = fy - (b / 200.0);
    const auto fx3 = fx * fx * fx;
    const auto xNormalised = (fx3 > e) ? fx3 : (116.0 * fx - 16.0) / kappa;
    const auto yNormalised = (l > ke) ? fy * fy * fy : (l / kappa);
    const auto fz3 = fz * fz * fz;
    const auto zNormalised = (fz3 > e) ? fz3 : (116.0 * fz - 16.0) / kappa;

    const auto x = xNormalised * k_whitePointD65[0];
    const auto y = yNormalised * k_whitePointD65[1];
    const auto z = zNormalised * k_whitePointD65[2];

    const auto rL = 3.2406 * x - 1.5372 * y - 0.4986 * z;
    const auto gL = -0.9689 * x + 1.8758 * y + 0.0415 * z;
    const auto bL = 0.0557 * x - 0.2040 * y + 1.0570 * z;

    const auto red = detail::delinearise(rL);
    const auto green = detail::delinearise(gL);
    const auto blue = detail::delinearise(bL);

    return Argb(red, green, blue);
}

} // namespace m3colours::quantise
