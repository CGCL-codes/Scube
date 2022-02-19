# This folder contains the source code files of the Scube

## How to use?
### Environment
We implement Scube in a Red Hat Enterprise Linux Server release 6.2 with an Intel 2.60GHz CPU and 64GB RAM, the size of one cache line in the server is 64 bytes. 

The g++ version we use is 7.3.0.

### Build
+ Commands
    1. ```make```
    2. ```make clean```

### Run
+ After executing the ```make``` command, we get the binary executable program "scube.r", and we can run it with the command ```./scube.r```. 

### Configurations
Some important parameters setting and theirs descriptions are as follows.
| Command-line parameters | Descriptions                                          |
|:----------------------- | :---------------------------------------------------- |
| **-dataset**            | choose dataset for testing                            |
| **-fplength**           | fingerprint length                                    |
| **-ignore_bits**        | the number of ignored bit in each hash value          |
| **-reserved_bits**      | the number of reserved bit in each hash value         |
| **-alpha**              | congestion factor                                     |
| **-exp**                | the expectation of critical degree                    |
| **-width**              | the width of the hash matrix                          |
| **-depth**              | the depth of the hash matrix                          |
| **-edgeweight**         | run edge weight query                                 |
| **-alledge**            | run edge weight query for all edges in the dataset    |
| **-edgeexistence**      | run edge existence query                              |
| **-nodeoutweight**      | run node-out aggregated weight query                  |
| **-nodeinweight**       | run node-in aggregated weight query                   |
| **-reachability**       | run reachability path query                           |
| **-kicks**              | kick times of each insertion                          |
| **-slots**              | the number of slots that the degree detector owns     |
| **-dataset_file_path**  | the file path of dataset                              |
| **-input_dir**          | the folder path of input files                        |
| **-output_dir**         | the folder path of output files                       |
| **-win**                | the granularity of the node degrees                   |
| **-write**              | output test results to file                           |

We give a simple example of how to run the scube with these parameters:
``` code
e.g. ./scube.r -dataset <int> -ignore_bits <int> -reserved_bits <int> -alpha <double> -exp <int> -fplength <int> -slots <int> -dataset_file_path <path> -width <int> -depth <int> -nodeinweight -input_dir <path> -output_dir <path>
e.g. ./scube.r -dataset 2 -ignore_bits 10 -reserved_bits 2 -alpha 0.844586 -exp 4779 -fplength 16 -slots 8191 -width 5658 -depth 5658 -nodeinweight -write -win 1000 -input_dir ../TestFiles/stk/input/node-1k-range-1k/ -output_dir ../TestFiles/stk/output/
```


## Authors and Copyright

Scube is developed in National Engineering Research Center for Big Data Technology and System, Cluster and Grid Computing Lab, Services Computing Technology and System Lab, School of Computer Science and Technology, Huazhong University of Science and Technology, Wuhan, China by Ming Chen (mingc@hust.edu.cn), Renxiang Zhou (mr\_zhou@hust.edu.cn), Hanhua Chen (chen@hust.edu.cn), Hai Jin (hjin@hust.edu.cn).

Copyright (C) 2022, [STCS & CGCL](http://grid.hust.edu.cn/) and [Huazhong University of Science and Technology](http://www.hust.edu.cn).

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
