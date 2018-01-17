import java.io.IOException;
import java.utils.StringTokenizer;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;

/* Map-Reduce phases:
 *
 * phase1 map:
 * (p, otherP) --> (p $ otherP, 1) 
 *
 * phase1 reduce:
 * (p $ otherP, 1) --> (p $ otherP, count_p_otherP) 
 *
 * phase2 map:
 * (p $ otherP, count_p_otherP) --> (p $ otherP, count_p_otherP $ count_p_otherP)
 *
 * phase2 reduce:
 * (p $ otherP, count_p_otherP, count_p_otherP) --> (p $ otherP, count_p $ count_p_otherP)
 *
 * phase3 map:
 * (p $ otherP, count_p $ count_p_otherP) --> (Rp(otherP), p $ otherP)
 *
 * phase3 reduce:
 * (Rp(otherP), p $ otherP) --> (p  otherP  Rp(otherP), nothing)
 * NOTE: does it only for the k first element (using a static array\counter)
 *
 * */

public class Solution {

    private static final Path INPUT_PATH = new Path(args[1]);
    private static final Path OUTPUT_PATH = new Path(args[2]);
    
    private static final Path INTER12_PATH = new Path("inter12");
    private static final Path INTER23_PATH = new Path("inter23");

    //-------------------------------------------------------------------------
    //                              Phase 1
    //-------------------------------------------------------------------------

    public static class TokenizerMapper extends
        Mapper<LongWritable, Text, Text, IntWriteable> {

            private final static IntWritable one = new IntWritable(1);
            private final Text twoProd = new Text();

            public void map(LongWritable key, Text val, Context context)
                throws IOException, InterruptedException {

                StringTokenizer st =
                    new StringTokenizer(val.toString().toLowerCase());
                twoProd.set(st.nextToken(",") + "$" + st.nextToken());
                context.write(twoProd, one);
            }
    }

    public static class IntSumReducer extends
        Reducer<Text, IntWritable, Text, IntWritable> {

            private final IntWritable result = new IntWritale();

            public void reduce(Text key, Iterable<IntWritable> vals, Context context)
                throws IOException, InterruptedException {

                int sum = 0;
                for (Text val : vals) {
                    StringTokenizer st =
                        new StringTokenizer(twoProds.get().toString());
                    sum += val;
                }
                result.set(sum);
                context.write(key, result);
            }

    } 

    //-------------------------------------------------------------------------
    //                              Phase 2
    //-------------------------------------------------------------------------







    /* @args[0] - k>0 defining how many product to report for each product
     * @args[1] - inputPath to the file that containe the data
     * @args[2] - outputPath to the result */
    public static void main(String[] args) throws Exception {
        
        System.out.println("args[0] = " + args[0]);
        System.out.println("args[1] = " + args[1]);
        System.out.println("args[2] = " + args[2]);
    }
}
