/**
 * @author Pippijn van Steenhoven
 */
public class binaersuche {
	private static boolean suche (int[] zahlen, int z, int links, int rechts) {
		int mitte = (links + rechts) / 2;
		return links > rechts
			? false
			: zahlen[mitte] == z
				? true
				: zahlen[mitte] > z
					? suche (zahlen, z, links, mitte - 1)
					: zahlen[mitte] < z
						? suche (zahlen, z, mitte + 1, rechts)
						: false;
		/*
		if (links > rechts)
			return false;
		if (zahlen[mitte] == z)
			return true;
		if (zahlen[mitte] > z)
			return suche (zahlen, z, links, mitte - 1);
		if (zahlen[mitte] < z)
			return suche (zahlen, z, mitte + 1, rechts);
		return false;
		*/
	}

	/**
	 * @param argv Arguments to the program
	 */
	public static void main (String[] argv) {
		int [] zahlen = { -1, 0, 1, 2, 3, 5, 6, 7, 8 };
		System.out.println (suche (zahlen, Integer.valueOf (argv[0]).intValue (), 0, zahlen.length - 1));
	}
}
