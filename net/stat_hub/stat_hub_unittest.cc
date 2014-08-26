/*
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "testing/gmock/include/gmock/gmock.h"
#include "gtest/gtest.h"

#include "stat_hub_cmd_api.h"
#include "stat_hub_cmd.h"
#include <map>

typedef std::multimap<unsigned int, StatHubCmd*> StatHubCmdMapType;

namespace unittest {


//   SH_CMD_GP_EVENT,                // 0
//   SH_CMD_WK_MAIN_URL,             // 1
//   SH_CMD_WK_MEMORY_CACHE,         // 2
//   SH_CMD_WK_RESOURCE,             // 3
//   SH_CMD_WK_PAGE,                 // 4
//   SH_CMD_WK_INSPECTOR_RECORD,     // 5
//   SH_CMD_WK_JS_SEQ,               // 6
//   SH_CMD_JAVA_GP_EVENT,           // 7
//   SH_CMD_CH_URL_REQUEST,          // 8
//   SH_CMD_CH_TRANS_NET,            // 9
//   SH_CMD_CH_TRANS_CACHE,          // 10
//   SH_CMD_TCPIP_SOCKET,            // 11
//   SH_CMD_PRELOADER                // 12


//static const char* main_url = "http://www.test_url.com";
// The fixture for testing class Foo.
class StatHubCmdAPITest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.
    StatHubCmdAPITest() {
        // You can do set-up work for each test here.
        //cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START, 0);
        //cmd->AddParamAsString(main_url);
        //cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_PAGE, SH_ACTION_WILL_START, 0);
    }

  virtual ~StatHubCmdAPITest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
  //StatHubCmd* cmd;
};

//TBD: disable for now. To be fixed later.
TEST_F(StatHubCmdAPITest, DISABLED_GetCmdMask){
  //default commands enabled when no plugin is registered
  unsigned int cmd_mask = 0;
  cmd_mask |= (1<<SH_CMD_WK_MEMORY_CACHE);
  cmd_mask |= (1<<SH_CMD_WK_MAIN_URL);
  EXPECT_EQ(cmd_mask, STAT_HUB_API(GetCmdMask)());
}

TEST_F(StatHubCmdAPITest, CmdCreate) {
    StatHubCmd* cmd;
    cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START, 145);
    ASSERT_NE((StatHubCmd*)0, cmd);
    EXPECT_EQ(SH_CMD_WK_MAIN_URL, cmd->GetCmd());
    EXPECT_EQ(SH_ACTION_WILL_START, cmd->GetAction());
    EXPECT_EQ((unsigned int)145, cmd->GetCookie());
    //release memory
    EXPECT_TRUE(STAT_HUB_API(CmdRelease)(cmd));
}

TEST_F(StatHubCmdAPITest, CmdCreateNotNeededByAnyProcessor) {
    StatHubCmd* cmd;
    cmd = STAT_HUB_API(CmdCreate)(SH_CMD_TBD_31, SH_ACTION_WILL_START, 0);
    ASSERT_EQ((StatHubCmd*)0, cmd);
}

TEST_F(StatHubCmdAPITest, CmdResetParams){
    StatHubCmd* cmd;
    unsigned int param = 17;
    cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START, 0);
    EXPECT_TRUE(cmd->params_.empty());                //the params should be empty by default
    cmd->AddParamAsUint32(param);
    EXPECT_EQ(1u,cmd->params_.size());                //no there should be single param
    EXPECT_EQ(param, cmd->GetParamAsUint32(0));
    EXPECT_TRUE(STAT_HUB_API(CmdResetParams)(cmd));    //reset
    EXPECT_TRUE(cmd->params_.empty());                //check that params are empty gain
    EXPECT_EQ(0u, cmd->GetParamAsUint32(0));            //check that zero is returned for no integer param
    //release memory
    EXPECT_TRUE(STAT_HUB_API(CmdRelease)(cmd));
}

TEST_F(StatHubCmdAPITest, CmdReleaseMoreThanOneRef){
    StatHubCmd* cmd;
    cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START, 0);
    ASSERT_NE((StatHubCmd*)0, cmd);
    EXPECT_EQ((unsigned int)1, cmd->referenced_);
    cmd->IncReference();
    cmd->IncReference();
    EXPECT_EQ((unsigned int)3, cmd->referenced_);
    EXPECT_TRUE(STAT_HUB_API(CmdRelease)(cmd));
    EXPECT_EQ((unsigned int)2, cmd->referenced_);
    EXPECT_TRUE(STAT_HUB_API(CmdRelease)(cmd));
    EXPECT_EQ((unsigned int)1, cmd->referenced_);
    EXPECT_TRUE(STAT_HUB_API(CmdRelease)(cmd));
}

TEST_F(StatHubCmdAPITest, CmdTimeStamp) {
    StatHubCmd* cmd;
    cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START, 0);
    ASSERT_NE((StatHubCmd*)0, cmd);
    ASSERT_TRUE(STAT_HUB_API(CmdTimeStamp)(cmd));
    StatHubTimeStamp ts1 = cmd->GetStartTimeStamp();
    usleep(3000); //sleep 3 msec
    ASSERT_TRUE(STAT_HUB_API(CmdTimeStamp)(cmd));
    StatHubTimeStamp ts2 = cmd->GetStartTimeStamp();
    EXPECT_GE(3,(ts2-ts1).InMilliseconds());
    EXPECT_TRUE(STAT_HUB_API(CmdRelease)(cmd));
}

TEST_F(StatHubCmdAPITest, CmdPushAndPop)
{
    unsigned int stack_size = 10;
    StatHubCmd* cmd = NULL;
    for (unsigned int i=1;i<=stack_size;i++) {
        cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START, i);
        ASSERT_NE((StatHubCmd*)0, cmd);
        EXPECT_TRUE(STAT_HUB_API(CmdPush)(cmd));
    }
    for (unsigned int i=1;i<=stack_size;i++) {
        //the following should fail
        cmd = STAT_HUB_API(CmdPop)(i,SH_CMD_WK_MEMORY_CACHE, SH_ACTION_WILL_START);
        EXPECT_EQ((StatHubCmd*)0, cmd);
        cmd = STAT_HUB_API(CmdPop)(i,SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_FINISH);
        EXPECT_EQ((StatHubCmd*)0, cmd);
        cmd = STAT_HUB_API(CmdPop)(0,SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START);
        EXPECT_EQ((StatHubCmd*)0, cmd);

        //this should succeed
        cmd = STAT_HUB_API(CmdPop)(i,SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START);
        ASSERT_NE((StatHubCmd*)0, cmd);
        EXPECT_EQ(i,cmd->GetCookie());
        ASSERT_TRUE(STAT_HUB_API(CmdRelease)(cmd));
    }

}

TEST_F(StatHubCmdAPITest, CmdParamCheck)
{
    //StatHubActionType max_action = SH_ACTION_LAST;
    StatHubCmd* cmdNULL = NULL;
    StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, SH_ACTION_WILL_START, 0);
    ASSERT_NE((StatHubCmd*)0, cmd);

    //TODO: check CmdCreate - any action is valid for now, shoudl we change it?
    //EXPECT_EQ((StatHubCmd*)0,STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, max_action, 0));
    //EXPECT_EQ((StatHubCmd*)0,STAT_HUB_API(CmdCreate)(SH_CMD_WK_MAIN_URL, (StatHubActionType)-1, 0));

    EXPECT_FALSE(STAT_HUB_API(CmdCommit)(cmdNULL));
    EXPECT_FALSE(STAT_HUB_API(CmdCommitDelayed)(cmdNULL,1u));
    EXPECT_FALSE(STAT_HUB_API(CmdCommitSync)(cmdNULL));
    EXPECT_FALSE(STAT_HUB_API(CmdPush)(cmdNULL));
    //EXPECT_FALSE(STAT_HUB_API(CmdRelease)(cmdNULL)); //TODO: should we return false on NULL?
    //EXPECT_FALSE(STAT_HUB_API(CmdResetParams)(cmdNULL));  //TODO: should we return false on NULL?
    EXPECT_FALSE(STAT_HUB_API(CmdTimeStamp)(cmdNULL));
}


//--------------------------------------------- StatHubCmd unit test -----------------------------

class MockStatHubCmd : public StatHubCmd {
private:
    bool* mock_destroyed_ptr_;
public:
    MockStatHubCmd(StatHubCmdType cmd, StatHubActionType action, unsigned int cookie,bool* mock_destroyed):
        StatHubCmd(cmd,action,cookie),mock_destroyed_ptr_(mock_destroyed) {
        if (mock_destroyed_ptr_)
            *mock_destroyed_ptr_ = false;
    }

    virtual ~MockStatHubCmd() {
        if (mock_destroyed_ptr_)
            *mock_destroyed_ptr_ = true;
    }
};

TEST(StatHubCmdTest, CmdInternals) {
    StatHubCmd cmd = StatHubCmd(SH_CMD_WK_MAIN_URL,SH_ACTION_WILL_START,5u);
    EXPECT_EQ(SH_CMD_WK_MAIN_URL,cmd.GetCmd());
    EXPECT_EQ(SH_ACTION_WILL_START,cmd.GetAction());
    EXPECT_EQ(5u,cmd.GetCookie());
    EXPECT_EQ(1u,cmd.referenced_);

    //params
    char buf[100] = "Testing testing 1,2,3";
    const char * str = (const char*) buf;
    void* ptr = &buf;
    cmd.AddParamAsBool(true);
    cmd.AddParamAsBool(false);
    cmd.AddParamAsBuf(buf,100u);
    cmd.AddParamAsPtr(ptr);
    cmd.AddParamAsString(str);
    cmd.AddParamAsUint32(120u);

    //fetching params
    unsigned int size=-1;
    void* newBuf = NULL;
    EXPECT_TRUE(cmd.GetParamAsBool(0));
    EXPECT_FALSE(cmd.GetParamAsBool(1));
    newBuf= cmd.GetParamAsBuf(2,size);
    EXPECT_EQ(100u,size);
    EXPECT_EQ(0,memcmp(buf,newBuf,size));
    EXPECT_EQ(ptr,cmd.GetParamAsPtr(3));
    EXPECT_STREQ(str,cmd.GetParamAsString(4));
    EXPECT_EQ(120u,cmd.GetParamAsUint32(5));

    //check for incorrect params
    size = 10u;
    EXPECT_FALSE(cmd.GetParamAsBool(6));
    EXPECT_EQ(NULL,cmd.GetParamAsBuf(6,size));
    EXPECT_EQ(0u,size);
    EXPECT_EQ(NULL,cmd.GetParamAsPtr(6));
    EXPECT_STREQ(NULL,cmd.GetParamAsString(6));
    EXPECT_EQ(0u,cmd.GetParamAsUint32(6));

    //reset params check
    cmd.ResetParams();
    EXPECT_FALSE(cmd.GetParamAsBool(0));

    //time stamp
    StatHubTimeStamp ts1 = StatHubTimeStamp::FromInternalValue(1000);
    StatHubTimeStamp ts2 = StatHubTimeStamp::FromInternalValue(5000);
    cmd.SetStartTimeStamp(ts1);
    cmd.SetCommitTimeStamp(ts2);
    EXPECT_EQ(ts2,cmd.GetCommitTimeStamp());
    EXPECT_EQ(ts1,cmd.GetStartTimeStamp());

    // ref count and release logic
    bool mock_destroyed = false; //assuming single thread nature for the gtest
    StatHubCmd* mock_cmd = new MockStatHubCmd(SH_CMD_WK_MAIN_URL,SH_ACTION_WILL_START,1u,&mock_destroyed);
    ASSERT_NE((void*)NULL,mock_cmd);
    mock_cmd->IncReference();
    StatHubCmd::Release(mock_cmd);
    ASSERT_FALSE(mock_destroyed);
    StatHubCmd::Release(mock_cmd);
    ASSERT_TRUE(mock_destroyed);
}


}//namespace unittest
