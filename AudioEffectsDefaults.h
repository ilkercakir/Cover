#ifndef AudioEffectsDefaultsH
#define AudioEffectsDefaultsH

#define REVERBDLINES 24

//float reverbprimes[REVERBDLINES] = {290.0, 387.0, 433.0, 470.0, 523.0, 559.0, 643.0};
//float reverbprimes[REVERBDLINES] = {307.0, 331.0, 353.0, 379.0, 401.0, 431.0, 449.0, 467.0};
//float reverbprimes[REVERBDLINES] = {293.0, 313.0, 347.0, 367.0, 383.0, 409.0, 431.0, 449.0, 463.0, 491.0, 509.0, 547.0, 569.0, 593.0, 613.0, 631.0};
float reverbprimes[REVERBDLINES] = {293.0, 307.0, 313.0, 331.0, 347.0, 367.0, 373.0, 383.0, 409.0, 419.0, 431.0, 449.0, 457.0, 463.0, 491.0, 509.0, 547.0, 557.0, 569.0, 577.0, 593.0, 613.0, 619.0, 631.0};

//           293    307    311    313    317    331    337    347    349 
//    353    359    367    373    379    383    389    397    401    409 
//    419    421    431    433    439    443    449    457    461    463 
//    467    479    487    491    499    503    509    521    523    541 
//    547    557    563    569    571    577    587    593    599    601 
//    607    613    617    619    631 

#define MAXCHORUS 3
float chofreq[MAXCHORUS] = {0.307, 0.409, 0.509}; // modulation frequencies
float chodepth[MAXCHORUS] = {0.003, 0.003, 0.003}; // modulation depths in percent 0 .. 1.0

#endif
