import java.io.IOException;
import java.util.StringTokenizer;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.Locale;
import java.math.RoundingMode;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.NullWritable;
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
 * (p$n$pk, count) --> (p$(1-count/n)$pk, none)
 * NOTE: the (1-frequence) is for enabling sorting in DECREASING order of the 
 *       frequency instead of INCREADING order because for 0.* numbers 
 *       lexicographic sort and value sort are the same.
 *
 * phase3 reduce:
 * (p$(1-count/n)$pk, [none, none, ..., none]) --> (p  pk  (count/n), none)
 * NOTE: does it only for the k first element (using a static array\counter)
 *
 * */

public class Solution {

    private static int k;
    private static Path INPUT_PATH;
    private static Path OUTPUT_PATH;
    
    private static final Path TEMP_PATH_12 = new Path("temp12");
    private static final Path TEMP_PATH_23 = new Path("temp23");

    private static final double THRESHOLD = 0.025;

    //-------------------------------------------------------------------------
    //                              Phase 1
    //-------------------------------------------------------------------------

    public static class DuplicateMapper extends
        Mapper<LongWritable, Text, Text, Text> {

            private static final Text prod1 = new Text();
            private static final Text prod2 = new Text();

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

            private static Text result = new Text();

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

            public void map(Text key, Text val, Context context)
                throws IOException, InterruptedException {

                String newKeyStr = new String();
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

            private static IntWritable result = new IntWritable();

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

    public static class ReverseFreqMapper extends
        Mapper<Text, Text, Text, NullWritable> {

            private static Text newKey = new Text();
            private final static NullWritable nothing = NullWritable.get();

            public void map(Text key, Text val, Context context)
                throws IOException, InterruptedException {

                String newKeyStr = new String();
                StringTokenizer st = new StringTokenizer(key.toString());
                int numOccurences;

                double pairOccurences  = Double.parseDouble(val.toString());

                newKeyStr = st.nextToken("$") + "$";

                int prodOccurences = Integer.parseInt(st.nextToken("$"));
                double reversedFreq = 1 - (pairOccurences / prodOccurences);
                String reversedFreqStr = String.valueOf(reversedFreq);

                newKeyStr += reversedFreqStr + "$";
                newKeyStr += st.nextToken();

                /* check threshold */
                if (1-reversedFreq >= THRESHOLD) {
                    newKey.set(newKeyStr);
                    context.write(newKey, nothing);
                }
            }
    }

    public static class ArrangeReducer extends
        Reducer<Text, NullWritable, Text, NullWritable> {

            private static Text newKey = new Text();
            private final static NullWritable nothing = NullWritable.get();

            /* keep only the K most relevant product */
            private static int counter = 1;
            private static String lastProd;

            public void reduce(Text key, Iterable<NullWritable> vals, Context context)
                throws IOException, InterruptedException {

                StringTokenizer st = new StringTokenizer(key.toString());
                String prod = new String(st.nextToken("$"));

                String newKeyStr = new String(prod + "\t");
                double reversedFreq = Double.parseDouble(st.nextToken("$"));
                newKeyStr += st.nextToken() + "\t";

                DecimalFormat formatter = new DecimalFormat("#.################",
                        DecimalFormatSymbols.getInstance( Locale.ENGLISH ));
                formatter.setRoundingMode( RoundingMode.DOWN );
                String freqStr = formatter.format(1-reversedFreq);

                newKeyStr += freqStr;

                if ( !prod.equals(lastProd) ) {
                    counter = 1;
                }

                if (counter <= k) {
                    newKey.set(newKeyStr);
                    context.write(newKey, nothing);
                    counter++;
                    lastProd = prod;
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
        if (!status2) {
            System.exit(1);
        }

        /* Clean temporary files from the previous MapReduce phase */
        fs.delete(TEMP_PATH_12, true);

        /* Setup third MapReduce phase */
        Job job3 = Job.getInstance(conf, "Ex4-third");
        job3.setJarByClass(Solution.class);
        job3.setMapperClass(ReverseFreqMapper.class);
        job3.setReducerClass(ArrangeReducer.class);
        job3.setMapOutputKeyClass(Text.class);
        job3.setMapOutputValueClass(NullWritable.class);
        job3.setOutputKeyClass(Text.class);
        job3.setOutputValueClass(NullWritable.class);
        job3.setInputFormatClass(KeyValueTextInputFormat.class);
        FileInputFormat.addInputPath(job3, TEMP_PATH_23);
        FileOutputFormat.setOutputPath(job3, OUTPUT_PATH);

        boolean status3 = job3.waitForCompletion(true);

        /* Clean temporary files from the first MapReduce phase */
        fs.delete(TEMP_PATH_23, true);

        if (!status3) {
            System.exit(1);
        }
    }
}


