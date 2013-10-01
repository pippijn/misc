/**
 * @author Pippijn van Steenhoven
 */
public class Timing {
	/**
	 * @param argv Arguments to the program
	 */
	public static void main (String[] argv) {
		long startTime, estimatedTime;

		startTime = System.nanoTime();
		for (int i = Integer.MIN_VALUE; i < 0; i++) {
		}
		estimatedTime = System.nanoTime() - startTime;

		System.out.println(estimatedTime);
	}
}
