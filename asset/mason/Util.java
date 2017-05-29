import sim.util.*;
import ec.util.*;

public class Util {
	public static double random(MersenneTwisterFast rng, double min, double max) {
		return min + rng.nextDouble() * (max - min);
	}

	public static Double2D random(MersenneTwisterFast rng, Double2D min, Double2D max) {
		return new Double2D(random(rng, min.x, max.x), random(rng, min.y, max.y));
	}

	public static Double3D random(MersenneTwisterFast rng, Double3D min, Double3D max) {
		return new Double3D(
			random(rng, min.x, max.x),
			random(rng, min.y, max.y),
			random(rng, min.z, max.z)
		);
	}
}
