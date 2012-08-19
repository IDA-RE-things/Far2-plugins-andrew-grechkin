/**
 * 
 */
package org.asd;

import java.math.BigDecimal;


/**
 * @author ander
 * 
 */
public class MainClass {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// Summarize.e();

		BigDecimal myValue = new BigDecimal("2");
		String rootOfTwo = Summarize.mySqrt(myValue, 100000);
		System.out.println("square root of " + myValue + " = " + rootOfTwo);
	}

	@Override
	public boolean equals(Object obj) {
		// TODO Auto-generated method stub
		return super.equals(obj);
	}

	@Override
	protected Object clone() throws CloneNotSupportedException {
		// TODO Auto-generated method stub
		return super.clone();
	}
}
