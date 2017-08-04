package example1;

import java.util.Arrays;
import java.util.Scanner;

/**
 * test date
 ********************/

public class Welcome {

	public static void main(String[] args) {
		Scanner inin = null;
		float aaa=2.95f;
		float bbb=2.39f;
		System.out.println("a+b:= "+(aaa+bbb));
		
		
		try {

			int[][] aa = new int[3][10];
			for (int i = 0; i < aa.length; i++) {
				for (int j = 0; j < aa[0].length; j++) {

					aa[i][j] = 2 * i + 5 * j + 10;
				}
			}

			for (int[] ele1 : aa) {
				for (int ele2 : ele1) {
					System.out.print(ele2 + " ");
				}
				System.out.print("\n");
			}
			System.exit(0);

			// int[] a={10,38,27,46,8899,100,1};
			int[] a = new int[] { 10, 38, 27, 46, 8899, 100, 1 };

			System.out.println("---1---" + a);
			for (int i : a)
				System.out.print(i + "| ");
			Arrays.sort(a);
			System.out.println("\n---2---" + a);
			for (int i : a)
				System.out.print(i + "| ");

			System.out.println("\n---1---WelcometoJAVA!!!!");
			System.out.println("---2---WelcometoJAVA!!!!");
			System.out.println("---3---WelcometoJAVA!!!!");
			System.out.println("---4---" + new abc().m_size);
			inin = new java.util.Scanner(System.in);
			while (inin.hasNextLine()) {
				System.out.println("input you message:");
				System.out.println(inin.next());
				if (inin.next().equals("q")) {
					System.out.println("Exit loop");
					System.exit(0);
				}
			}

		} catch (Exception e) {

		} finally {

			inin.close();
		}

	}

	/**
	 * test2
	 * 
	 * 
	 */
	public int getsize() {

		return 0;
	}

}

class abc {
	public int m_size = 100;
	public byte i = 10;
}

class def{
	/**
     * Writes the specified byte to this stream.  If the byte is a newline and
     * automatic flushing is enabled then the <code>flush</code> method will be
     * invoked.
     *
     * <p> Note that the byte is written as given; to write a character that
     * will be translated according to the platform's default character
     * encoding, use the <code>print(char)</code> or <code>println(char)</code>
     * methods.
     * @author abc
     * @param  b The byte to be written
     * @see #print(char)
     * @see #println(char)
     * @see #println(char,int)
     */
	public void  init(){
		
	}
	
	
	
	
}
