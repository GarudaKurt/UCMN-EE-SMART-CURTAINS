#ifndef __MOTORCONTROL__H
#define __MOTORCONTROL__H

class MOTORCONTROL {
  public:
    MOTORCONTROL();
    ~MOTORCONTROL();
    void initStepper();
    void downWard(float temp,  float humid, float light);
    void upWard(float temp, float humid, float light);
    void stopMotor();
    void manualForwardStep();
    void manualReverseStep();

  private:
};

#endif