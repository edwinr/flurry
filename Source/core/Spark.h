// Spark.h: interface for the Spark class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SPARK_H)
#define SPARK_H

typedef struct Spark {
    float position[3];
    int mystery;
    float delta[3];
    float color[4];
} Spark;

void UpdateSparkColour(Spark* s,
                       double fTime,
                       int currentColorMode,
                       double flurryRandomSeed);
void InitSpark(Spark* s);
void UpdateSpark(Spark* s,
                 double fTime,
                 double fDeltaTime,
                 int currentColorMode,
                 double flurryRandomSeed);

#endif  // !defined(SPARK_H)
