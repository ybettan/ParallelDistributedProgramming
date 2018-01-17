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
    
    private static final Path TEMP_PATH = new Path("temp");
    //private static final Path INTER12_PATH = new Path("inter12");
    //private static final Path INTER23_PATH = new Path("inter23");

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




    //-------------------------------------------------------------------------
    //                              Phase 3
    //-------------------------------------------------------------------------




    //-------------------------------------------------------------------------
    //                               Main
    //-------------------------------------------------------------------------

    /* @args[0] - k>0 defining how many product to report for each product
     * @args[1] - inputPath to the file that containe the data
     * @args[2] - outputPath to the result */
    public static void main(String[] args) throws Exception {
        
        Configuration conf = new Configuration();
        FileSystem fs = FileSystem.get(conf);

        /* Just to be safe: clean temporary files before we begin */
        fs.delete(TEMP_PATH, true);

        /* We chain the 3 Mapreduce phases using a temporary directory
         * from which the first phase writes to, and the second reads from.
         * then the second writes to and the third read from */

        /* Setup first MapReduce phase */
        Job job1 = Job.getInstance(conf, "Ex4-first");
        job1.setJarByClass(Solution.class);
        job1.setMapperClass(TokenizerMapper.class);
        job1.setReducerClass(IntSumReducer.class);
        job1.setMapOutputKeyClass(Text.class);
        job1.setMapOutputValueClass(IntWritable.class);
        job1.setOutputKeyClass(Text.class);
        job1.setOutputValueClass(IntWritable.class);
        FileInputFormat.addInputPath(job1, new Path(args[1]));
        FileOutputFormat.setOutputPath(job1, TEMP_PATH);

        boolean status1 = job1.waitForCompletion(true);
        if(!status1) {
            System.exit(1);
        }

        ///* Setup second MapReduce phase */
        //Job job2 = Job.getInstance(conf, "WordOrder-second");
        //job2.setJarByClass(WordOrder.class);
        //job2.setMapperClass(SwapMapper.class);
        //job2.setReducerClass(OutputReducer.class);
        //job2.setMapOutputKeyClass(IntWritable.class);
        //job2.setMapOutputValueClass(Text.class);
        //job2.setOutputKeyClass(Text.class);
        //job2.setOutputValueClass(NullWritable.class);
        //job2.setInputFormatClass(KeyValueTextInputFormat.class);
        //FileInputFormat.addInputPath(job2, TEMP_PATH);
        //FileOutputFormat.setOutputPath(job2, new Path(args[1]));
        //
        //boolean status2 = job2.waitForCompletion(true);
        //
        ///* Clean temporary files from the first MapReduce phase */
        //fs.delete(TEMP_PATH, true);

        //if (!status2) System.exit(1);

    }
}


