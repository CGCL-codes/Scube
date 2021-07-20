# Scube: Efficient Summarization for Skewed Graph Stream
Scube is an efficient summarization structure for skewed graph stream. Two factors contribute to the efficiency of Scube. First, Scube explores a novel probabilistic counting based scheme to efficiently identify the high-degree nodes in graph stream. 
The proposed scheme effectively avoids the influence of the recurring edges by only recording the appearance of the patterns of the neighbors' hash values instead of counting the patterns. Second, based on the identified high-degree nodes, Scube uses a dynamic address allocation strategy to assign more rows or columns in the compressive matrix to the high-degree nodes and thus alleviate the hash collisions. 

## How to use?
### Environment
We implement Scube in a Red Hat Enterprise Linux Server release 6.2 with an Intel 2.60GHz CPU and 64GB RAM, the size of one cache line in the server is 64 bytes. 

The g++ version we use is 7.3.0.

Build

```txt
make
./scube
```

### Configurations
Some important parameters setting and theirs descriptions are as follows.
| Command-line parameters | Descriptions                                       |
|:----------------------- | :------------------------------------------------- |
| **-width**              | the width of the hash matrix                       |
| **-depth**              | the depth of the hash matrix                       |
| **-fplength**           | fingerprint length                                 | 
| **-edgeweight**         | run edge weight query                              |
| **-edgeexistence**      | run edge existence query                           |
| **-nodeinweight**       | run node-in aggregated weight query                |
| **-nodeoutfrequence**   | run node-out aggregated weight query               |
| **-reachability**       | run reachability path query                        |
| **-rpq**                | run reachability path query while inserting tuples |
| **-kicks**              | kick times of each insertion                       |
| **-k_width**            | the threshold of width                             |
| **-k_depth**            | the threshold of depth                             |
| **-slots**              | the number of slots that the degree detector owns  |
| **-filename**           | the file path of dataset                           |
| **-input_dir**          | the folder path of input files                     |
| **-output_dir**         | the folder path of output files                    |

We give a simple example of how to use these parameters:
``` code
e.g. ./tcm -filename <path> -width <int> -depth <int> -hashnum <int> -nodeinweight -input_dir <path> -output_dir <path>
e.g. ./gss -filename <path> -width <int> -depth <int> -range <int> -candidate <int> -slot <int> -fplength <int> -edgeweight -input_dir <path> -output_dir <path>
e.g. ./scube -filename <path> -width <int> -depth <int> -fplength <int> -slots <int> -k_width <int> -k_depth <int> -reachability -input_dir <path> -output_dir <path>
```


## Author and Copyright

Scube is developed in National Engineering Research Center for Big Data Technology and System, Cluster and Grid Computing Lab, Services Computing Technology and System Lab, School of Computer Science and Technology, Huazhong University of Science and Technology, Wuhan, China by Ming Chen (mingc@hust.edu.cn), Renxiang Zhou (mr\_zhou@hust.edu.cn), Hanhua Chen (chen@hust.edu.cn), Hai Jin (hjin@hust.edu.cn).

Copyright (C) 2021, [STCS & CGCL](http://grid.hust.edu.cn/) and [Huazhong University of Science and Technology](http://www.hust.edu.cn).
