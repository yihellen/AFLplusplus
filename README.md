# Combining Grammar Based Mutation Strategies Gramatron and Nautilus

`Gramatron` and `Nautilus` are two grammar aware fuzzers that run on top of American Fuzzy Lop (AFL). `Gramatron` and `Nautilus` keep different internal representations of the their inputs’ syntactic structures. `Gramatron` represents the inputs it generates as automaton walks, and it mutates its inputs more aggressively. `Nautilus` represents its inputs as parse trees and conducts more localized changes during mutation. This project attempts to combine these two fuzzers and test if these two different styles of mutation can lead to more efficient fuzzing.

## Experiment Environment
- Pull the docker image from docker-hub and run it
```
docker pull yihellen/combine-gramatron-nautilus:github_demo
```

## Run sample experiments
### Build seed generator and shared library files on different grammars for different targets
- Each fuzz target is run with a different subset of the input program's grammar that is relevant towards triggering a certain bug to keep the experiment time reasonable. These grammars files are stored under `/root/gramatron-artifact/grammars`.
For example, the grammar files under the `mruby-1` directory are listed as follows[^1] :
```
mruby-1
 ┣ mruby1.json            // Required for building the shared library file for Nautilus
 ┣ source.g4            
 ┣ source_automata.json   // Finite state automaton (FSA) representation of the grammar required for Gramatron
 ┣ source_nt_gh.py       
 ┗ source_nt_p.json
```

- All the seed generators and shared library files required to run `Nautilus` (aka `grammar_mutator` in the experiment docker image) are already available in `/root/gramatron-artifact/fuzzers/AFLplusplus/custom_mutators/grammar_mutator/grammar_mutator`. If you want to build these files again your self, run `python3 /root/gramatron-artifact/fuzzers/AFLplusplus/custom_mutators/grammar_mutator/grammar_mutator/generate-grammar-so.py`.

### Run experiments with Beanstalk queue
- Deploy a Beanstalk server which will act as a job queue manager
```
beanstalkd &
```
- All the config files used in this project are readily available in `/root/gramatron-artifact/experiments/gt_bugs`. You can also generate the config files yourself. In `/root/gramatron-artifact/experiments/gt_bugs/generate_json_files.py`, keep the fuzz targets that you want to run against in `lang_to_target` (comment out the targets that you don't wish to run)[^2]. Use the different argument to specify what experiments to run. `generate_json_files.py` has the following options:
```
optional arguments:
  -h, --help   show this help message and exit
  --combine    run combined Gramatron and Nautilus in one AFLplusplus fuzz
               session
  --pipeline   run Gramatron and Nautilus in pipeline
  --gramatron  run gramatron
  --nautilus   run nautilus
```
For instance, to generate the config file for fuzzing all available fuzz targets using gramatron only, run
```
python3 generate_json_files.py --gramatron
```
- Flush Beanstalk queue at least three times
```
cd /root/gramatron-artifact/experiments/gt_bugs
python3.6 run_ground_truth_campaign.py -c <config files> --numcores <# of cores> --flush
```
You need to specify the config file as well as the number of cores to use in this command. It is recommended to use the number of cores available - 1 as the number of cores to use here. For example, if your computer has 15 cores, it is recommended to set `--numcores` to 14.
- Put the jobs onto the queue 
```
python3.6 run_ground_truth_campaign.py -c <config files> --numcores <# of cores> --put
```
- Run the jobs
```
python3.6 run_ground_truth_campaign.py -c <config files> --numcores <# of cores> --get
```


[^1]: Different versions of Nautilus accepts grammer files written in different formats. In this project, I used the Nautilus implemetation from https://github.com/AFLplusplus/Grammar-Mutator/tree/ff4e5a265daf5d88c4a636fb6a2c22b1d733db09, whose input grammar format is different from another Nautilus implementation which can be found here: https://github.com/nautilus-fuzz/nautilus. Python file `/root/gramatron-artifact/fuzzers/AFLplusplus/custom_mutators/grammar_mutator/grammar_mutator/grammars/nautilus_py_grammar_to_json.py` converts grammar files accepted by the latter to ones that are acceptable by the former. See https://github.com/yihellen/AFLplusplus/tree/stable/custom_mutators/gramatron for more information on how to convert grammar files into its FSA representation. 

[^2]: When running the combined Gramatron and Nautilus in one AFLplusplus session in my experiment, only `mruby-1`, `mruby-2`, `JS-1` and `JS-4` are used as fuzz targets. This is because the different grammar files for `JS-3`, `PHP-1` and `PHP-2` inside `/root/gramatron-artifact/grammars` represent slightly different grammars of the input language which causes the automaton parser to fail. Future work to unify these different formats of grammar is needed to run all the available fuzz targets in this docker image.
