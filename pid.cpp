#include "PID.h"
// #include <iostream>

ModelPID::ModelPID()
    : k{}
    , tI{}
    , tD{}
    , I_result{}
    , D_result{}
    , pamiecCalk{0.0}
    , pamiecRozn{0.0}
{}

ModelPID::ModelPID(double tempK, double tempTI, double tempTD) : k{tempK}, tI{tempTI}, tD{tempTD}, pamiecCalk{0.0}, pamiecRozn{0.0} {

}

ModelPID::~ModelPID(){

}

void ModelPID::ustawK(double tempK){
    k = tempK;
}

void ModelPID::ustawTI(double tempTI){
    tI = tempTI;
}

void ModelPID::ustawTD(double tempTD){
    tD = tempTD;
}

void ModelPID::resetujPamiecCalk(){
    pamiecCalk = 0.0;
}

void ModelPID::resetujPamiecRozn(){
    pamiecRozn = 0.0;
}

void ModelPID::resetujPamiec(){
    resetujPamiecCalk();
    resetujPamiecRozn();
}

void ModelPID::dostosujPamiec(bool isToggled) {
    if (isToggled) {
        pamiecCalk *= tI;
    }
    else {
        pamiecCalk /= tI;
    }
}

double ModelPID::czescProp(double eI){
    return k * eI;
}

double ModelPID::czescCalk_ConstOut(double eI){
    if (tI != 0) {
        pamiecCalk += eI;
        I_result = pamiecCalk / tI;
        return pamiecCalk / tI;
    }
    else
        return 0.0;
}

double ModelPID::czescCalk_ConstIn(double eI){
    if (tI != 0)
        pamiecCalk += eI / tI;

    I_result = pamiecCalk;
    return pamiecCalk;
}

double ModelPID::czescRozn(double eI){
    double temp = tD * (eI - pamiecRozn);
    pamiecRozn = eI;
    D_result = temp;
    return temp;
}

double ModelPID::symulujKrokPID_IConstOut(double eI){
    double p_, i_, d_;
    p_ = czescProp(eI);
    i_ = czescCalk_ConstOut(eI);
    d_ = czescRozn(eI);
    PID_result = p_ + i_ + d_;
    return p_ + i_ + d_;
}

double ModelPID::symulujKrokPID_IConstIn(double eI){
    double p_, i_, d_;
    p_ = czescProp(eI);
    i_ = czescCalk_ConstIn(eI);
    d_ = czescRozn(eI);
    PID_result = p_ + i_ + d_;
    return p_ + i_ + d_;
}

