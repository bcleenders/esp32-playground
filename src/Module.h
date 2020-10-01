// Avoid importing multiple times
#ifndef MODULE_H_DEFINED
#define MODULE_H_DEFINED

class Module
{
public:
    virtual void run_main() = 0;
    virtual void run_loop() = 0;
};

#endif
