import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.DoubleWritable;
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
 * (count/n, [p$pk1, p$pk2, ...,p$pkn) --> (p  pk1  count/n, none) 
 *                                         (p  pk2  count/n, none) 
 *                                         ...
 *                                         (p  pkn  count/n, none) 
 *
 * NOTE: does it only for the k first element (using a static array\counter)
 *
 * */

public class Solution {

    private static int k;
    private static Path INPUT_PATH;
    private static Path OUTPUT_PATH;
    
    private static final Path TEMP_PATH_12 = new Path("temp12");
    private static final Path TEMP_PATH_23 = new Path("temp23");

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

    public static class CountMapper extends
        Mapper<Text, Text, Text, IntWritable> {

            private final static IntWritable one = new IntWritable(1);
            private static Text newKey = new Text();
            private static String newKeyStr = new String();

            public void map(Text key, Text val, Context context)
                throws IOException, InterruptedException {

                StringTokenizer st = new StringTokenizer(val.toString());
                int numOfOtherProducts = 0;

                /* count the number of products that were selled with the key */
                for (int i=0 ; i<val.toString().length() ; i++) {
                    if (val.toString().charAt(i) == '$')
                        numOfOtherProducts++;
                }
                numOfOtherProducts++;

                while (st.hasMoreTokens()) {
                    newKeyStr = key.toString() + "$" + (numOfOtherProducts) +
                        "$" + st.nextToken("$");
                    newKey.set(newKeyStr);
                    context.write(newKey, one);
                }
            }
    }

    public static class CountReducer extends
        Reducer<Text, IntWritable, Text, IntWritable> {

            private final IntWritable result = new IntWritable();

            public void reduce(Text key, Iterable<IntWritable> vals, Context context)
                throws IOException, InterruptedException {

                int sum = 0;
                for (IntWritable val : vals) {
                    sum += val.get();
                }

                result.set(sum);
                context.write(key, result);
            }
    } 

    //-------------------------------------------------------------------------
    //                              Phase 3
    //-------------------------------------------------------------------------

    public static class FreqMapper extends
        Mapper<Text, Text, DoubleWritable, Text> {

            private static String newValStr = new String();
            private static DoubleWritable newKey = new DoubleWritable();
            private static Text newVal = new Text();

            public void map(Text key, Text val, Context context)
                throws IOException, InterruptedException {

                StringTokenizer st = new StringTokenizer(key.toString());
                int numOccurences;

                String tmp = new String();
                tmp = val.toString();
                double newKeyDouble  = Double.parseDouble(tmp);

                newValStr = st.nextToken("$") + "$";
                numOccurences = Integer.parseInt(st.nextToken("$"));
                newValStr += st.nextToken();

                newKey.set(newKeyDouble / numOccurences);
                newVal.set(newValStr);
                context.write(newKey, newVal);
            }
    }

    public static class ArrangeReducer extends
        Reducer<DoubleWritable, Text, Text, NullWritable> {

            private Text newKey = new Text();
            private String newKeyStr = new String();
            private final static NullWritable nothing = NullWritable.get();

            public void reduce(DoubleWritable key, Iterable<Text> vals, Context context)
                throws IOException, InterruptedException {

                StringTokenizer st;

                for (Text val : vals) {
                    st = new StringTokenizer(val.toString());
                    newKeyStr = st.nextToken("$") + "\t" + st.nextToken();
                    newKeyStr += "\t" + Double.toString(key.get());

                    newKey.set(newKeyStr);
                    context.write(newKey, nothing);
                }

            }
    } 

    //-------------------------------------------------------------------------
    //                               Main
    //-------------------------------------------------------------------------

    /* @args[0] - k>0 defining how many product to report for each product
     * @args[1] - inputPath to the file that containe the data
     * @args[2] - outputPath to the result */
    public static void main(String[] args) throws Exception {

        k = Integer.parseInt(args[0]);
        INPUT_PATH = new Path(args[1]);
        OUTPUT_PATH = new Path(args[2]);
        
        Configuration conf = new Configuration();
        FileSystem fs = FileSystem.get(conf);

        /* Just to be safe: clean temporary files before we begin */
        fs.delete(TEMP_PATH_12, true);
        fs.delete(TEMP_PATH_23, true);
        fs.delete(OUTPUT_PATH, true);

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
        FileOutputFormat.setOutputPath(job1, TEMP_PATH_12);

        boolean status1 = job1.waitForCompletion(true);
        if(!status1) {
            System.exit(1);
        }

        /* Setup second MapReduce phase */
        Job job2 = Job.getInstance(conf, "Ex4-second");
        job2.setJarByClass(Solution.class);
        job2.setMapperClass(CountMapper.class);
        job2.setReducerClass(CountReducer.class);
        job2.setMapOutputKeyClass(Text.class);
        job2.setMapOutputValueClass(IntWritable.class);
        job2.setOutputKeyClass(Text.class);
        job2.setOutputValueClass(IntWritable.class);
        job2.setInputFormatClass(KeyValueTextInputFormat.class);
        FileInputFormat.addInputPath(job2, TEMP_PATH_12);
        FileOutputFormat.setOutputPath(job2, TEMP_PATH_23);
        
        boolean status2 = job2.waitForCompletion(true);
        if (!status2) System.exit(1);

        /* Clean temporary files from the previous MapReduce phase */
        fs.delete(TEMP_PATH_12, true);

        /* Setup third MapReduce phase */
        Job job3 = Job.getInstance(conf, "Ex4-third");
        job3.setJarByClass(Solution.class);
        job3.setMapperClass(FreqMapper.class);
        job3.setReducerClass(ArrangeReducer.class);
        job3.setMapOutputKeyClass(DoubleWritable.class);
        job3.setMapOutputValueClass(Text.class);
        job3.setOutputKeyClass(Text.class);
        job3.setOutputValueClass(NullWritable.class);
        job3.setInputFormatClass(KeyValueTextInputFormat.class);
        FileInputFormat.addInputPath(job3, TEMP_PATH_23);
        FileOutputFormat.setOutputPath(job3, OUTPUT_PATH);
        
        boolean status3 = job3.waitForCompletion(true);
        if (!status3) System.exit(1);
        
        /* Clean temporary files from the first MapReduce phase */
        fs.delete(TEMP_PATH_23, true);

    }
}


