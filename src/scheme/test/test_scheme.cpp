#include  "scheme.hpp"

#include <gtest/gtest.h>
#include <vector>

#include <fmt/core.h>

namespace samos::scheme {

class TestScheme : public ::testing::Test
{
protected:
    TestScheme()
        :
        schemer{}
    {
    }

    Schemer schemer;
};

TEST_F(TestScheme, TestCarCdr)
{
    sexp null_sexp = SEXP_NULL;

    ASSERT_NO_THROW(schemer.car(null_sexp));
    ASSERT_NO_THROW(schemer.cdr(null_sexp));
    ASSERT_NO_THROW(schemer.car_cdr(null_sexp));
}

TEST_F(TestScheme, TestAssq)
{
    schemer.import_module(SrfiType::Srfi_ListLibrary);
    SchemerResult<sexp> sexp_res = schemer.read_from_file("data/tests/test_assq.scm");

    ASSERT_TRUE(sexp_res.is_ok());

    sexp file_contents = sexp_res.get_ok();

    ASSERT_EQ(schemer.sexp_type(file_contents), SexpType::Pair);

    sexp_res = schemer.assq("test-cfg", file_contents);
    ASSERT_TRUE(sexp_res.is_ok());

    sexp_res = schemer.assq("fake", file_contents);
    ASSERT_TRUE(sexp_res.is_err());
}

// FIXME: Failing.
#if 0
TEST_F(TestScheme, TestImportModule)
{
    samos::log::logger::set_level(samos::log::logger::LogLevel::Debug);
    std::vector<SchemeModule> modules{
        SrfiType::Srfi_0,
        SrfiType::Srfi_1,
        SrfiType::Srfi_ListLibrary,
        SrfiType::Srfi_2,
        SrfiType::Srfi_6,
        SrfiType::Srfi_8,
        SrfiType::Srfi_9,
        SrfiType::Srfi_11,
        SrfiType::Srfi_14,
        SrfiType::Srfi_16,
        SrfiType::Srfi_18,
        // SrfiType::Srfi_22,
        SrfiType::Srfi_23,
        SrfiType::Srfi_26,
        SrfiType::Srfi_27,
        SrfiType::Srfi_33,
        SrfiType::Srfi_38,
        SrfiType::Srfi_39,
        SrfiType::Srfi_41,
        SrfiType::Srfi_46,
        SrfiType::Srfi_55,
        // SrfiType::Srfi_62,
        SrfiType::Srfi_69,
        SrfiType::Srfi_95,
        SrfiType::Srfi_98,
        SrfiType::Srfi_99,
        SrfiType::Srfi_101,
        SrfiType::Srfi_111,
        SrfiType::Srfi_113,
        SrfiType::Srfi_115,
        SrfiType::Srfi_116,
        SrfiType::Srfi_117,
        SrfiType::Srfi_121,
        SrfiType::Srfi_124,
        SrfiType::Srfi_125,
        SrfiType::Srfi_127,
        SrfiType::Srfi_128,
        SrfiType::Srfi_129,
        SrfiType::Srfi_130,
        SrfiType::Srfi_132,
        SrfiType::Srfi_133,
        SrfiType::Srfi_134,
        SrfiType::Srfi_135,
        SrfiType::Srfi_139,
        SrfiType::Srfi_141,
        SrfiType::Srfi_142,
        SrfiType::Srfi_143,
        SrfiType::Srfi_144,
        SrfiType::Srfi_145,
        SrfiType::Srfi_147,
        SrfiType::Srfi_151,
        SrfiType::Srfi_154,
        SrfiType::Srfi_158,
        // SrfiType::Srfi_160,
        SrfiType::Srfi_165,
        SrfiType::Srfi_166,
        SrfiType::Srfi_188,
        ChibiModule::ChibiApp,
        ChibiModule::ChibiAst,
        ChibiModule::ChibiBase64,
        ChibiModule::ChibiBytevector,
        ChibiModule::ChibiConfig,
        ChibiModule::ChibiCrypto,
        ChibiModule::ChibiCryptoMd5,
        ChibiModule::ChibiCryptoRsa,
        ChibiModule::ChibiCryptoSha2,
        ChibiModule::ChibiDiff,
        ChibiModule::ChibiDisasm,
        ChibiModule::ChibiDoc,
        ChibiModule::ChibiEditdistance,
        ChibiModule::ChibiEquiv,
        ChibiModule::ChibiFilesystem,
        ChibiModule::ChibiGeneric,
        ChibiModule::ChibiHeap,
        ChibiModule::ChibiIo,
        ChibiModule::ChibiIsetBase,
        ChibiModule::ChibiIsetConstructors,
        ChibiModule::ChibiIsetIterators,
        ChibiModule::ChibiJson,
        ChibiModule::ChibiLoop,
        ChibiModule::ChibiMatch,
        ChibiModule::ChibiMathPrime,
        ChibiModule::ChibiMemoize,
        ChibiModule::ChibiMime,
        ChibiModule::ChibiModules,
        ChibiModule::ChibiNet,
        ChibiModule::ChibiNetHttpServer,
        ChibiModule::ChibiNetServlet,
        ChibiModule::ChibiParse,
        ChibiModule::ChibiPathname,
        ChibiModule::ChibiProcess,
        ChibiModule::ChibiRepl,
        ChibiModule::ChibiScribble,
        ChibiModule::ChibiString,
        ChibiModule::ChibiStty,
        ChibiModule::ChibiSxml,
        ChibiModule::ChibiSystem,
        ChibiModule::ChibiTempFile,
        ChibiModule::ChibiTest,
        ChibiModule::ChibiTime,
        ChibiModule::ChibiTrace,
        ChibiModule::ChibiType,
        ChibiModule::ChibiUri,
        ChibiModule::ChibiWeak,
    };

    for (auto module: modules)
    {
        // Schemer schemer2;
        EXPECT_TRUE(schemer.import_module(module).is_ok());
    }
}
#endif

}
