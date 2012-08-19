/**
 * 
 */
package org.asd;

import java.math.BigDecimal;
import java.math.MathContext;
import java.math.RoundingMode;

/**
 * @author ander
 *
 */
class Summarize {
	public long add(long left, long right) {
		return left + right;
	}

	public static void e() {
		BigDecimal e = new BigDecimal("2");
		BigDecimal fac = new BigDecimal("1");

		int cycles = 2000;

		for (int j = 2; j < cycles; j++) {
			fac = fac.multiply(new BigDecimal(j));

			BigDecimal facInv = new BigDecimal("1");

			MathContext mc = new MathContext(cycles, RoundingMode.HALF_EVEN);
			facInv = facInv.divide(fac, mc);

			e = e.add(facInv);
			e.setScale(cycles, BigDecimal.ROUND_HALF_EVEN);
		}

		System.out.println("e =  " + e.toString().substring(0, 2 + cycles));
	}

	public static String mySqrt(BigDecimal x, int digits) {
		BigDecimal r = x;
		BigDecimal rPrev = x;
		BigDecimal half = new BigDecimal("0.5");

		MathContext mc = new MathContext(digits, RoundingMode.HALF_EVEN);

		BigDecimal accuracy = new BigDecimal("1");
		BigDecimal tenth = new BigDecimal("0.1");
		for (int i = 0; i < digits; ++i) {
			accuracy = accuracy.multiply(tenth);
		}

		final long minIterations = 20;
		long iterations = 0;

		String result = r.toString();

		// abs(r*r - x) > accuracy && abs(r - rPrev) > accuracy
		while (iterations < minIterations
				|| minIterations <= iterations
				&& (r.multiply(r, mc).subtract(x, mc).abs(mc)
						.compareTo(accuracy) > 0)
				&& (r.subtract(rPrev, mc).abs(mc).compareTo(accuracy) > 0)) {

			BigDecimal tmp = r;
			// r = (r + x / r) / 2
			r = r.add(x.divide(r, mc), mc).multiply(half, mc);
			rPrev = tmp;

			result = r.toString();
			System.out.println("it = " + iterations + " temp result = "
					+ result);

			++iterations;
		}

		return result;
	}
}
