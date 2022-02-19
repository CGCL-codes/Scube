# This project contains two baselines
## Code
+ TCM
+ GSS
----
The code of TCM and GSS in experiment of the paper is downloaded from https://github.com/Puppy95/Graph-Stream-Sketch provided by the GSS authors. 

For more detailed information, please refer to the website.

## Build
+ Commands
    1. ```make```
    2. ```make clean```

## Run
+ After executing the ```make``` command, we get the two executable programs, tcm and gss, which can be run with commands ```./tcm``` or ```./gss```. 

### Configurations
Some important parameters setting and theirs descriptions are as follows.
| Command-line parameters | Descriptions                                          |
|:----------------------- | :---------------------------------------------------- |
| **-hashnum**            | the number of different hash functions (only for tcm) |
| **-range**              | the number of positions for each node (only for gss)  |
| **-candidate**          | the number of positions for each edge (only for gss)  |
| **-slot**               | the number of entries for each bucket (only for gss)  |
| **-fplength**           | fingerprint length (only for gss)                     | 
| **-dataset**            | choose dataset for testing                            |
| **-width**              | the width of the hash matrix                          |
| **-depth**              | the depth of the hash matrix                          |
| **-edgeweight**         | run edge weight query                                 |
| **-alledge**            | run edge weight query for all edges in the dataset    |
| **-edgeexistence**      | run edge existence query                              |
| **-nodeoutweight**      | run node-out aggregated weight query                  |
| **-nodeinweight**       | run node-in aggregated weight query                   |
| **-reachability**       | run reachability path query                           |
| **-dataset_file_path**  | the file path of dataset                              |
| **-input_dir**          | the folder path of input files                        |
| **-output_dir**         | the folder path of output files                       |
| **-win**                | the granularity of the node degrees                   |
| **-write**              | output test results to file                           |

We give a simple example of how to run the scube with these parameters:
``` code
e.g. ./tcm -dataset <int> -dataset_file_path <path> -hashnum <int> -width <int> -depth <int> -nodeinweight -input_dir <path> -output_dir <path>
e.g. ./tcm -dataset 2 -hashnum 6 -nodeinweight -write -width 5658 -depth 5658 -win 1000 -input_dir ../TestFiles/stk/input/node-1k-range-1k/ -output_dir ../TestFiles/stk/output/

e.g. ./gss -dataset <int> -dataset_file_path <path> -range <int> -candidate <int> -slot <int> -fplength <int> -width <int> -depth <int> -nodeinweight -input_dir <path> -output_dir <path>
e.g. ./gss -dataset 2 -range 4 -candidate 16 -slot 2 -fplength 16 -width 5658 -depth 5658 -nodeinweight -write -win 1000 -input_dir ../TestFiles/stk/input/node-1k-range-1k/ -output_dir ../TestFiles/stk/output/
```