# MaxEnt source code review

Source code review of Tsuruoka's C++ library for maximum entropy classification (see [http://www.nactem.ac.uk/tsuruoka/maxent/](http://www.nactem.ac.uk/tsuruoka/maxent/)).


## Introduction

On his website, [Tsuruoka](http://www.logos.ic.i.u-tokyo.ac.jp/~tsuruoka/) proposed a C++ library for maximum entropy classification. In order to get a better and deeper understanding of implementation details, I propose here a simple code review. The code base is relatively small (around 2500 lines of code). Those notes are primary destined for my personal use and reflect my current understanding. I propose them here, in case where that could help someone.

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

The `mathvec.h` describes the class `Vec` which extends the possibilities of `vector<double>`:  the operators (e.g. `<<`, binary `+`, etc.) are overloaded and some facilities are in place for computing the dot product and the vector projection. Those elements are used in `lbfgs.cpp`, `owlqn.cpp` and `maxent.h`.

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


## The core

The class `ME_Model` is the core of the library. This is reflected in the space  that class takes in `maxent.h` (about 80% of the written source code). A couple of nested structures are defined inside `ME_Model`. Note that nesting does not imply other relations than a shared scope for the nested structures. Inheritance or composition are not involved, unless they are explicitly stated. The following figure gives an insight about the global architecture:

![maxent.h](./resources/maxent_h.png)


### `ME_Sample` and  `Sample`

The structure `ME_Sample` records the observations. Each `ME_Sample` have its  own features. There are two kinds of features: binary (`vector<string>`) and real-valued (`vector<pair<string, double>>`) features. 

When the `ME_Sample` records are added to the training set, they are transformed in `Sample`: a distinction is made here between the _external observations_ (`ME_Sample`) and the _internal representation of those observations_ (`Sample`), inaccessible outside the `ME_Model`.

Since it's easier to handle integers in place of strings, a common trick () is to map each string label (and each features) to an integer identifier using an hash_map:


![hash_table](./resources/hash_table.png)

Once done, it's easier to refer and manipulate the labels and the features through matrices. Note that in some algorithm, such as SVM, the value of identifier can be directly used in the computation.


### Hash implementation

By default, pre-processor instructions in `maxent.h` call the header `ext/hash_map`. This implementation of hash table was [one the first largely used](https://en.wikipedia.org/wiki/Unordered_associative_containers_%28C%2B%2B%29#History), with `hash_set`, `hash_multimap` and `hash_multiset`. However, this is now outdated and kept  only for downgraded compatibility. Thus, compiling the project using this kind of hash lead to a warning with a C++11 compiler. As indicated in the source, the best way to skip the warning is to comment the macro which defines `USE_HASH_MAP`.

```C++
#define USE_HASH_MAP  // if you encounter errors with hash, try commenting out this line.  (the program will be a bit slower, though)
#ifdef USE_HASH_MAP
#include <ext/hash_map>
#endif
```

The comments indicates that the program will be a bit slower. This is expected since the standard library container `map` orders its keys.

```C++
#ifdef USE_HASH_MAP
    typedef __gnu_cxx::hash_map<unsigned int, int> map_type;
#else    
    typedef std::map<unsigned int, int> map_type;
#endif
```

A good alternative would be to use an [unordered_map](https://en.wikipedia.org/wiki/Unordered_associative_containers_%28C%2B%2B%29) (maybe a pull request?). The C++ Technical Report 1 ([TR1](https://en.wikipedia.org/wiki/C%2B%2B_Technical_Report_1)), whose improvements contributed to C++11, suggests that way. If you are interested, check out this article: [Hash Table Performance Tests](http://preshing.com/20110603/hash-table-performance-tests/).



### Objective function

The objective function, which we want optimize, is a log-likelihood function:

![equation](https://qph.is.quoracdn.net/main-qimg-273326bfff461974edc1802b550b2507?convert_to_webp=true)

In the library, this is done using the following code:

```C++
    double
ME_Model::update_model_expectation()
{
    double logl = 0;
    int ncorrect = 0;

    _vme.resize(_fb.Size());
    for (int i = 0; i < _fb.Size(); i++) _vme[i] = 0;

    int n = 0;
    for (vector<Sample>::const_iterator i = _vs.begin(); i != _vs.end(); i++, n++) {
        vector<double> membp(_num_classes);
        int max_label = conditional_probability(*i, membp);

        logl += log(membp[i->label]);
        //    cout << membp[*i] << " " << logl << " ";
        if (max_label == i->label) ncorrect++;

        // model_expectation
        for (vector<int>::const_iterator j = i->positive_features.begin(); j != i->positive_features.end(); j++){
            for (vector<int>::const_iterator k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++) {
                _vme[*k] += membp[_fb.Feature(*k).label()];
            }
        }
        for (vector<pair<int, double> >::const_iterator j = i->rvfeatures.begin(); j != i->rvfeatures.end(); j++) {
            for (vector<int>::const_iterator k = _feature2mef[j->first].begin(); k != _feature2mef[j->first].end(); k++) {
                _vme[*k] += membp[_fb.Feature(*k).label()] * j->second;
            }
        }

    }

    for (int i = 0; i < _fb.Size(); i++) {
        _vme[i] /= _vs.size();
    }

    _train_error = 1 - (double)ncorrect / _vs.size();

    logl /= _vs.size();

    if (_l2reg > 0) {
        const double c = _l2reg;
        for (int i = 0; i < _fb.Size(); i++) {
            logl -= _vl[i] * _vl[i] * c;
        }
    }

    //logl /= _vs.size();

    //  fprintf(stderr, "iter =%3d  logl = %10.7f  train_acc = %7.5f\n", iter, logl, (double)ncorrect/train.size());
    //  fprintf(stderr, "logl = %10.7f  train_acc = %7.5f\n", logl, (double)ncorrect/_train.size());

    return logl;
}
```


```C++
int
ME_Model::conditional_probability(const Sample & s,
        std::vector<double> & membp) const
{
    //int num_classes = membp.size();
    double sum = 0;
    int max_label = -1;
    //  double maxp = 0;

    vector<double> powv(_num_classes, 0.0);
    for (vector<int>::const_iterator j = s.positive_features.begin(); j != s.positive_features.end(); j++){
        for (vector<int>::const_iterator k = _feature2mef[*j].begin(); k != _feature2mef[*j].end(); k++) {
            powv[_fb.Feature(*k).label()] += _vl[*k];
        }
    }
    for (vector<pair<int, double> >::const_iterator j = s.rvfeatures.begin(); j != s.rvfeatures.end(); j++) {
        for (vector<int>::const_iterator k = _feature2mef[j->first].begin(); k != _feature2mef[j->first].end(); k++) {
            powv[_fb.Feature(*k).label()] += _vl[*k] * j->second;
        }
    }

    std::vector<double>::const_iterator pmax = max_element(powv.begin(), powv.end());
    double offset = max(0.0, *pmax - 700); // to avoid overflow
    for (int label = 0; label < _num_classes; label++) {
        double pow = powv[label] - offset;
        double prod = exp(pow);
        //      cout << pow << " " << prod << ", ";
        //      if (_ref_modelp != NULL) prod *= _train_refpd[n][label];
        if (_ref_modelp != NULL) prod *= s.ref_pd[label];
        assert(prod != 0);
        membp[label] = prod;
        sum += prod;
    }
    for (int label = 0; label < _num_classes; label++) {
        membp[label] /= sum;
        if (membp[label] > membp[max_label]) max_label = label;
    }
    assert(max_label >= 0);
    return max_label;
}
```







## Optimization methods

By default, this library use 'LBFGS' to optimize the objective.



## Example of postagging




