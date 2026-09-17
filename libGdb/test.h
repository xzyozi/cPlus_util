
#include <iostream>
#include <sstream>
// #define private public

#include <Cmn0030ConnectionFactory.h>
#include <Cmn0030InfConnection.h>

namespace TEST_CMN0030
{
    const std::map<bool, std::string> mBoolStr = {
        {true,  "true" },
        {false, "false"},
    };

    void testCmn0030Connection(void);

    void testCmn0030Dto(void);

} // namespace TEST_CMN0020
