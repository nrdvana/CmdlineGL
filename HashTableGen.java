import java.io.*;
import java.util.*;

/**
 * <p>Project: 42 Programming System</p>
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2004-2005</p>
 *
 * @author Michael Conrad
 * @version $Revision$
 */
public class HashTableGen {
	static final int hashFunc(String key, int mod) {
		int mask= mod-1;
		byte[] bytes= key.getBytes();
		int result= 0;
		for (int i=0; i<bytes.length; i++)
			result= (result<<4) + (result ^ (bytes[i]&0xFF)) & mask;
		return result;
	}
    public static void main(String[] args) {
		try {
			if (args.length != 4) {
				System.out.println("Usage: java HashTableGen BucketCount TableName BucketName EntryStruct < table_data > TabeData.c");
				System.exit(-1);
			}
			Vector[] table = new Vector[Integer.parseInt(args[0])];
			String TableName= args[1];
			String BucketName= args[2];
			String EntryStruct= args[3];

			LineNumberReader lineIn= new LineNumberReader(new InputStreamReader(System.in));
			String line= null;
			try {
				while ((line= lineIn.readLine()) != null) {
					if (line.startsWith("DATA")) {
						StringTokenizer st= new StringTokenizer(line, "\t ");
						st.nextToken(); // DATA
						String key= st.nextToken();
						String value= st.nextToken("");
						int hash= hashFunc(key, table.length);
						if (table[hash] == null) table[hash]= new Vector();
						table[hash].add(new Object[] { key, value });
					}
				}
			}
			catch (Exception ex) {
				throw new RuntimeException("Error on line "+lineIn.getLineNumber()+" of input stream.",  ex);
			}
			PrintWriter out= new PrintWriter(System.out);
			for (int i=0; i<table.length; i++) {
				if (table[i] != null) {
					out.print(EntryStruct+" "+BucketName+i+"["+table[i].size()+"] = {");
					for (int ent= 0; ent < table[i].size(); ent++) {
						Object[] entry= (Object[]) table[i].get(ent);
						if (ent != 0) out.print(", ");
						out.print("{\""+entry[0].toString()+"\", "+entry[1].toString()+" }");
					}
					out.println("};");
				}
			}
			out.println("struct "+TableName+"Bucket { int EntryCount; "+EntryStruct+" *Entries; }; ");
			out.println("#define "+TableName+"Size "+table.length);
			out.println("#define EMPTY {0, 0}");
			out.println("const struct "+TableName+"Bucket "+TableName+"["+table.length+"] = {");
			for (int i=0; i<table.length; i++) {
				if ((i&0x07) == 0) out.print("\t");
				if (table[i] != null)
					out.print("{"+table[i].size()+", "+BucketName+i+"}");
				else
					out.print("EMPTY");
				if (i != table.length-1)
					out.print(", ");
				if ((i&0x07) == 7)
					out.println();
			}
			out.println("};");
			out.println("#undef EMPTY");
			out.flush();
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
    }
}