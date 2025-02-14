// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>
#include "src/App/InitApplication.h"

#include <memory>
#include <QString>

#include <Base/Parameter.h>
#include <Mod/Start/App/ThumbnailSource.h>

class ThumbnailSourceOverrider: public Start::ThumbnailSource
{
public:
    ThumbnailSourceOverrider(QString file, ParameterGrp::handle hGrp)
        : Start::ThumbnailSource(file, hGrp)
    {}

    std::tuple<QString, QStringList> wrapCreateF3DCall(const QString& thumbnailPath) const
    {
        return createF3DCall(thumbnailPath);
    }
};


class ThumbnailSourceTest: public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        tests::initApplication();
    }

    void SetUp() override
    {
        QLatin1String testPath {"/foo/bar/baz"};
        _parameterGroup = ParameterManager::Create();
        _parameterGroup->Manager()->CreateDocument();
        _parameterGroup->SetASCII("f3d", "fake_f3d");
        _source = std::make_shared<ThumbnailSourceOverrider>(testPath, _parameterGroup);
    }

    void TearDown() override
    {}

    std::shared_ptr<ThumbnailSourceOverrider> Source()
    {
        return _source;
    }

    Base::Reference<ParameterGrp> Group()
    {
        return _parameterGroup;
    }


private:
    std::shared_ptr<ThumbnailSourceOverrider> _source;
    Base::Reference<ParameterGrp> _parameterGroup;
};

TEST_F(ThumbnailSourceTest, f3dSetup)
{
    // Arrange - done in Init()

    // Act
    auto [f3d, args] = Source()->wrapCreateF3DCall("5860a733ec3759be22a3473cc26f71d5.png");

    // Assert
    EXPECT_EQ(f3d.toStdString(), Group()->GetASCII("f3d"));
    EXPECT_TRUE(args.contains(QLatin1String("--output=5860a733ec3759be22a3473cc26f71d5.png")));
}
