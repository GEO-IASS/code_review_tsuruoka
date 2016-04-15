/*
 *
 * Simple example
 * 
 */

#include "maxent.h"
#include <iostream>

int main(){

    ME_Sample s1("CLASS_A");
    s1.add_feature("f1");             
    s1.add_feature("f2"); 
    s1.add_feature("f3", 4.0);  

    ME_Sample s2("CLASS_A");
    s2.add_feature("f2");             
    s2.add_feature("f3", 5.0);  
    s2.add_feature("f4", 1.0); 

    ME_Sample s3("CLASS_B");
    s3.add_feature("f1");             
    s3.add_feature("f3", 1.0);  
    s3.add_feature("f4", 3.0); 


    ME_Model model;
    model.add_training_sample(s1);
    model.add_training_sample(s2);
    model.add_training_sample(s3);

    model.explore();

    model.train();

}
