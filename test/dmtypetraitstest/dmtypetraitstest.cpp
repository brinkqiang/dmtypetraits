
#include "dmtypetraits.h"
#include "gtest.h"

class env_dmtypetraits
{
public:
    void init(){}
    void uninit(){}
};

class frame_dmtypetraits : public testing::Test
{
public:
    virtual void SetUp()
    {
        env.init();
    }
    virtual void TearDown()
    {
        env.uninit();
    }
protected:
    env_dmtypetraits env;
};

TEST_F(frame_dmtypetraits, init)
{
    Idmtypetraits* module = dmtypetraitsGetModule();
    if (module)
    {
        module->Test();
        module->Release();
    }
}
