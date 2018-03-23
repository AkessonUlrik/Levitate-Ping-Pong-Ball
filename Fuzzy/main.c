#include <stdio.h>
#include <math.h>

// define a max function that returns the biggest value of a and b
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// defines a min function that returns the smallest value of a and b
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int main() {
    // Declare and initialize variables
    double error = 0, deltaError = 0;
    double normError = 0, normDeltaError = 0;
    double  eN = 0, eZ = 0, eP = 0, deN = 0, deZ = 0, deP = 0;
    double ruleP = 0, ruleZ = 0, ruleN = 0;
    double areaP = 0, areaZ = 0, areaN = 0;
    double deltaAngle = 0;
    double alpha = asin(1.0/6.0);

    //For debugging purposes the error and deltaError is given by the user, not to be used in the final version
    printf("Enter error: ");
    scanf("%lf", &error);
    printf("Enter change in error: ");
    scanf("%lf", &deltaError);

    //Normalize the the error and deltaError with the following function (e-min)/(max-min) max and min value for error is +/-20 and for deltaError is +/-10
    normError = (error+20)/40;
    normDeltaError = (deltaError+10)/20;

    //printf debugging, not used in the final code
    printf("e = %lf, nE = %lf, dE = %lf, ndE = %lf\n",error, normError, deltaError, normDeltaError);

    //Fuzzyfication of the error based on the membership function of the fuzzy set for the error eN = negative, eZ = zero, eP = positive
    if (error <= -10){
        eN = 1;
    } else if (error > -10 && error < 0){
        eN = -4*(normError-0.25)+1;
        eZ = 4*(normError-0.25);
    } else if (error == 0){
        eZ = 1;
    } else if (error > 0 && error < 10){
        eZ = -4*(normError-0.5)+1;
        eP = 4*(normError-0.5);
    } else if (error >= 0){
        eP = 1;
    }

    //printf debugging, not used in the final code
    printf("eN = %lf, eZ = %lf, eP = %lf\n", eN, eZ, eP);

    //Fuzzyfication of the deltaError based on the membership function of the fuzzy set for the error deN = negative, deZ = zero, deP = positive
    if (deltaError <= -6){
        deN = 1;
    } else if (deltaError > -6 && deltaError < 0){
        deN = -(3+(1.0/3.0))*(normDeltaError-0.2)+1;
        deZ = (3+(1.0/3.0))*(normDeltaError-0.2);
    } else if (deltaError == 0){
        deZ = 1;
    } else if (deltaError > 0 && deltaError < 6){
        deZ = -(3+(1.0/3.0))*(normDeltaError-0.5)+1;
        deP = (3+(1.0/3.0))*(normDeltaError-0.5);
    } else if (deltaError >= 6){
        deP = 1;
    }

    //printf debugging, not used in the final code
    printf("deN = %lf, deZ = %lf, deP = %lf\n", deN, deZ, deP);

    //rule matching with min and max as operator
    ruleP = max(max(min(eN, deN), min(eN, deZ)), min(eZ, deP));
    ruleZ = max(max(min(eN, deP), min(eZ, deZ)), min(eP, deN));
    ruleN = max(max(min(eZ, deP), min(eP, deZ)), min(eP, deP));

    //printf debugging, not used in the final code
    printf("rP = %lf, rZ = %lf, rN = %lf\n", ruleP, ruleZ, ruleN);

    //Calculate the area of the implied fuzzy sets.
    areaP = ((6*ruleP-((1-ruleP)/tan(alpha))*ruleP)/2)+((1-ruleP)/tan(alpha))*ruleP;
    areaZ = ((12*ruleZ-((1-ruleZ)/tan(alpha))*ruleZ)/2)+((1-ruleZ)/tan(alpha))*ruleZ;
    areaN = ((6*ruleN-((1-ruleN)/tan(alpha))*ruleN)/2)+((1-ruleN)/tan(alpha))*ruleN;

    //printf debugging, not used in the final code
    printf("aP = %lf, aZ = %lf, aN = %lf\n", areaP, areaZ, areaN);

    //Defuzzification, or rather my version of it. Seems to work as intended, however not the same as on the exam...
    if (areaZ >= 1) {
        deltaAngle = 0 + (areaP / areaZ) * 6 - (areaN / areaZ) * 6;
        printf("aZ\n");
    } else if (areaN >= 1){
        deltaAngle = -6 + (areaZ / areaN) * 6 + (areaP / areaN) * 12;
        printf("aN\n");
    } else if (areaP >= 1){
        deltaAngle = 6 - (areaZ / areaP) * 6 - (areaN / areaP) * 12;
        printf("aP\n");
    } else {
        printf("Unknown case! \n");
    }
    //printf debugging, not used in the final code
    printf("deltaAnge = %lf, round delatAngle = %lf\n", deltaAngle, round(deltaAngle));
    return 0;
}