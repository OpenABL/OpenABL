import sim.util.*;
import ec.util.*;

public class Util {
	public static double random(MersenneTwisterFast rng, double min, double max) {
		return min + rng.nextDouble() * (max - min);
	}
}
