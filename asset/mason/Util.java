import sim.util.*;
import ec.util.*;
import java.util.*;
import java.io.*;
import java.lang.reflect.*;

public class Util {
	public static double random(MersenneTwisterFast rng, double min, double max) {
		return min + rng.nextDouble() * (max - min);
	}

  public static int randomInt(MersenneTwisterFast rng, int min, int max) {
    return min + rng.nextInt(max - min + 1);
  }

	private static void saveAgent(PrintWriter writer, Object agent, Class<?> cls)
			throws IllegalAccessException {
		writer.print("{");
		boolean first = true;
		for (Field field : cls.getDeclaredFields()) {
			if (field.getName().startsWith("_dbuf_")) continue;

			if (!first) writer.print(",");
			first = false;
			writer.format("\"%s\":", field.getName());

			Class<?> fieldCls = field.getType();
			if (fieldCls.equals(boolean.class)) {
				boolean b = field.getBoolean(agent);
				writer.print(b ? "true" : "false");
			} else if (fieldCls.equals(int.class)) {
				int i = field.getInt(agent);
				writer.print(i);
			} else if (fieldCls.equals(double.class)) {
				double f = field.getDouble(agent);
				writer.print(f);
			} else if (fieldCls.equals(Double2D.class)) {
				Double2D f = (Double2D) field.get(agent);
				writer.format("[%f,%f]", f.x, f.y);
			} else if (fieldCls.equals(Double3D.class)) {
				Double3D f = (Double3D) field.get(agent);
				writer.format("[%f,%f,%f]", f.x, f.y, f.z);
			} else {
				assert false;
			}
		}
		writer.print("}");
	}

	private static void saveAgents(PrintWriter writer, ArrayList<Object> agents, Class<?> cls)
			throws IllegalAccessException {
		writer.print("[");

		boolean first = true;
		for (Object agent : agents) {
			if (!first) writer.print(",\n");
			first = false;
			saveAgent(writer, agent, cls);
		}
		writer.print("]");
	}

	public static void save(Bag bag, String fileName) {
		// Get agents separated by type
		HashMap<String, ArrayList<Object>> agentsMap = new HashMap<>();
		for (int i = 0; i < bag.size(); i++) {
			Object agent = bag.get(i);
			String className = agent.getClass().getName();
			ArrayList<Object> list = agentsMap.get(className);
			if (list == null) {
				list = new ArrayList<>();
				agentsMap.put(className, list);
			}
			list.add(agent);
		}

		try {
			PrintWriter writer = new PrintWriter(fileName, "UTF-8");
			writer.print("{");
			boolean first = true;
			for (HashMap.Entry<String, ArrayList<Object>> entry : agentsMap.entrySet()) {
				if (!first) writer.print(",");
				first = false;

				String className = entry.getKey();
				Class<?> cls = Class.forName(className);
				ArrayList<Object> agents = entry.getValue();
				writer.format("\"%s\":", className);
				saveAgents(writer, agents, cls);
			}
			writer.print("}");
			writer.close();
		} catch (Exception e) {
			System.out.format("save(): Count not open \"%s\" for writing\n", fileName);
		}
	}
}
