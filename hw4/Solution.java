import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;

import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.KeyValueTextInputFormat;
import org.apache.hadoop.fs.FileSystem;

/* Map-Reduce phases:
 *
 * phase1 map:
 * (lineNum, p1,p2) --> (p1, p2)
 *                      (p2, p1)
 *
 * phase1 reduce:
 * (p, [p1, p2, ..., pn]) --> (p, p1$p2$...$pn) 
 *
 * phase2 map:
 * (p, p1$p2$...$pn) --> (p$n$p1, 1)
 *                       (p$n$p2, 1)
 *                       ...
 *                       (p$n$pn, 1)
 *
 * phase2 reduce:
 * (p$n$pk, [1, 1, ..., 1]) --> (p$n$pk, count)
 *
 * phase3 map:
 * (p$n$pk, count) --> (count/n, p$pk)
 *
 * phase3 reduce:
 * (count/n, p$pk) --> (p  pk  count/n, none) 
 * NOTE: does it only for the k first element (using a static array\counter)
 *
 * */

public class Solution {

    private static Path INPUT_PATH;
    private static Path OUTPUT_PATH;
    
    private static final Path TEMP_PATH = new Path("temp");

    //-------------------------------------------------------------------------
    //                              Phase 1
    //-------------------------------------------------------------------------

    public static class DuplicateMapper extends
        Mapper<LongWritable, Text, Text, Text> {

            private final Text prod1 = new Text();
            private final Text prod2 = new Text();

            public void map(LongWritable key, Text val, Context context)
                throws IOException, InterruptedException {

                StringTokenizer st =
                    new StringTokenizer(val.toString().toLowerCase());
                prod1.set(st.nextToken(","));
                prod2.set(st.nextToken());
                context.write(prod1, prod2);
                context.write(prod2, prod1);
            }
    }

    public static class ConcatReducer extends
        Reducer<Text, Text, Text, Text> {

            private Text result = new Text();

            public void reduce(Text key, Iterable<Text> vals, Context context)
                throws IOException, InterruptedException {

                String str_result = new String();
                for (Text val : vals) {
                    str_result += "$" + val.toString();
                }
                /* cut the firs "$" in the result */
                result.set(str_result.substring(1));
                context.write(key, result);
            }

    } 

    //-------------------------------------------------------------------------
    //                              Phase 2
    //-------------------------------------------------------------------------

    //public static class DuplicateMapper extends
    //    Mapper<LongWritable, Text, Text, IntWritable> {

    //        private final static IntWritable one = new IntWritable(1);
    //        private final Text twoProd = new Text();

    //        public void map(LongWritable key, Text val, Context context)
    //            throws IOException, InterruptedException {

    //            StringTokenizer st =
    //                new StringTokenizer(val.toString().toLowerCase());
    //            twoProd.set(st.nextToken(",") + " $ " + st.nextToken());
    //            context.write(twoProd, one);
    //        }
    //}

    //public static class ConcatReducer extends
    //    Reducer<Text, IntWritable, Text, IntWritable> {

    //        private final IntWritable result = new IntWritable();

    //        public void reduce(Text key, Iterable<IntWritable> vals, Context context)
    //            throws IOException, InterruptedException {

    //            int sum = 0;
    //            for (IntWritable val : vals) {
    //                sum += val.get();
    //            }
    //            result.set(sum);
    //            context.write(key, result);
    //        }

    //} 



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

        final int K = Integer.parseInt(args[0]);
        INPUT_PATH = new Path(args[1]);
        OUTPUT_PATH = new Path(args[2]);
        
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
        job1.setMapperClass(DuplicateMapper.class);
        job1.setReducerClass(ConcatReducer.class);
        job1.setMapOutputKeyClass(Text.class);
        job1.setMapOutputValueClass(Text.class);
        job1.setOutputKeyClass(Text.class);
        job1.setOutputValueClass(Text.class);
        FileInputFormat.addInputPath(job1, INPUT_PATH);
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


