# MaxEnt source code review

Source code review of Tsuruoka's C++ library for maximum entropy classification (see [http://www.nactem.ac.uk/tsuruoka/maxent/](http://www.nactem.ac.uk/tsuruoka/maxent/)).


## Introduction

On his website, [Tsuruoka](http://www.logos.ic.i.u-tokyo.ac.jp/~tsuruoka/) proposed a C++ library for maximum entropy classification. In order to get a better and deeper understanding of implementation details, I propose here a simple code review. The code base is relatively small (around 2500 lines of code).
Those notes are primary destined for my personal use and reflect my current understanding. I propose them here, in case where that could help someone.

This article is divided in three parts:

 - The core
 - Optimization methods
 - Postagging example

Before digging ahead, let's get an overview of the code base.


## Overview

Using [Rexdep](https://github.com/itchyny/rexdep), we can obtain the following directed graph of dependencies:

![Dependencies](https://raw.githubusercontent.com/fauconnier/code_review_tsuruoka/master/resources/dependencies.png)

The __core of the library__ are the header `maxent.h`, and its implementation `maxent.cpp`. In this core, the fundamental structures and functions of the learning task are defined (training set struct, functions for computing of objective, etc.). The following section  will discuss some of these aspects.

Besides that, three __optimization methods__ are proposed:

 - `lbfgs.cpp`: the very efficient [BFGS algorithm](https://en.wikipedia.org/wiki/Broyden–Fletcher–Goldfarb–Shanno_algorithm) in limited memory version;
 - `owlqn.cpp`: a [variant of BFGS](https://en.wikipedia.org/wiki/Limited-memory_BFGS#OWL-QN) for L1-regularized models;
 - `sgd.cpp`: the classic [stochastic gradient descent](https://en.wikipedia.org/wiki/Stochastic_gradient_descent);

Note that, due to their shared nature, `lbfgs.cpp` and `owlqn.cpp` include both the header `lbfgs.h`.

Two __classification examples__ are proposed:

 - `bicycle.cpp`: binary classification between cars and bicycles given simple features. This example is made to just figure out the basic principles;
 - `postaggin.cpp`: the classic task which aims at identifying the right part-of-speech (POS) for a given token. A little dataset of 200 sentences is given to train and test new models;

### A word about `mathvec.h`

The `mathvec.h` describes the class `Vec` which extends the possibilities of `vector<double>`:  the operators (e.g. `<<`, binary `+`, etc.) are overloaded and some facilities are in place for computing the dot product and the vector projection. Those elements are extensively used in `lbfgs.cpp`, `owlqn.cpp` and `maxent.h`.

In this case, the optimization aspect is taken into account. For instance, the dot production function is [inlined](https://en.wikipedia.org/wiki/Inline_function), in order to save CPU registers and increase the efficience of operation.

```C++
inline double dot_product(const Vec & a, const Vec & b)
{
  double sum = 0;
  for (size_t i = 0; i < a.Size(); i++) {
    sum += a[i] * b[i];
  }
  return sum;
}
```

### A word about `ext/hash_map`

By default, pre-processor instructions in `maxent.h` call the header `ext/hash_map`. This implementation of hash table was [one the first largely used](https://en.wikipedia.org/wiki/Unordered_associative_containers_%28C%2B%2B%29#History), with `hash_set`, `hash_multimap` and `hash_multiset`. However, this is now outdated and kept for only for historical reasons. Thus, compiling the project using this kind of hash lead to a warning with a C++11 compiler. As indicated in the source, the best way to skip the warning is to comment the macro which defines `USE_HASH_MAP`.

```C++
#define USE_HASH_MAP  // if you encounter errors with hash, try commenting out this line.  (the program will be a bit slower, though)
#ifdef USE_HASH_MAP
#include <ext/hash_map>
#endif
```

The comments indicates that the program will be a bit slower. This is expected since the standard library container `map`, which ordered its keys, will be used in place of `ext/hash_map`. 

```C++
#ifdef USE_HASH_MAP
    typedef __gnu_cxx::hash_map<unsigned int, int> map_type;
#else    
    typedef std::map<unsigned int, int> map_type;
#endif
```

A good alternative would be to use an [unordered_map](https://en.wikipedia.org/wiki/Unordered_associative_containers_%28C%2B%2B%29). The C++ Technical Report 1 ([TR1](https://en.wikipedia.org/wiki/C%2B%2B_Technical_Report_1)), whose improvements contributed to C++11, suggests that way. For those interested in obtaining further information, please refer the following blog entry: [Hash Table Performance Tests](http://preshing.com/20110603/hash-table-performance-tests/).


## The core


The class `ME_Model` is the core of the library. This is reflected in the space  that class takes in `maxent.h` (about 80% of the written source code). A couple of nested structures are defined inside `ME_Model`. Note that nesting does not imply other relations than a shared scope for the nested structures. Inheritance or composition are not involved, unless they are explicitly stated. In order to access to nested objects, it is necessary to instantiate them inside the stuperstructure. The following figure gives an insight about the architecture of `ME_Model`:


![maxent.h](./resources/maxent_h.png)


The structure `ME_Sample` records the samples. Each sample keeps a record of its features. There are two kinds of features: discrete and real-valued features. The set of discrete features is `vector<string>`, whereas the real-valued features are pairs `vector<pair<string, double>>`.


```C++
struct ME_Sample
{
public:
  ME_Sample() : label("") {}; // Constructors
  ME_Sample(const std::string & l) : label(l) {}; // Constructors
  void set_label(const std::string & l) { label = l; } // Setters

  // to add a binary feature
  void add_feature(const std::string & f) {
    features.push_back(f);   
  }

  // to add a real-valued feature
  void add_feature(const std::string & s, const double d) {
    rvfeatures.push_back(std::pair<std::string, double>(s, d)); 
  }

public:
  std::string label;
  std::vector<std::string> features;
  std::vector<std::pair<std::string, double> > rvfeatures;

  // obsolete
  void add_feature(const std::pair<std::string, double> & f) {  
    rvfeatures.push_back(f); // real-valued features
  }
};
```



### Features




## Optimization methods




## POSTagging




