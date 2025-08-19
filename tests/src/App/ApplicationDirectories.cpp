#include <gtest/gtest.h>
#include <App/ApplicationDirectories.h>

#include <random>

namespace fs = std::filesystem;

static fs::path MakeUniqueTempDir()
{
    constexpr int maxTries = 128;
    const fs::path base = fs::temp_directory_path();
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;

    for (int i = 0; i < maxTries; ++i) {
        auto name = "app_directories_test-" + std::to_string(dist(gen));
        fs::path candidate = base / name;
        std::error_code ec;
        if (fs::create_directory(candidate, ec)) {
            return candidate;
        }
        if (ec && ec != std::make_error_code(std::errc::file_exists)) {
            continue;
        }
    }
    throw std::runtime_error("Failed to create unique temp directory");
}

class ApplicationDirectoriesTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        _tempDir = MakeUniqueTempDir();
    }

    std::map<std::string, std::string>
    generateConfig(const std::map<std::string, std::string>& overrides) const
    {
        std::map<std::string, std::string> config {{"AppHomePath", _tempDir.string()},
                                                   {"ExeVendor", "Vendor"},
                                                   {"ExeName", "Test"},
                                                   {"BuildVersionMajor", "4"},
                                                   {"BuildVersionMajor", "2"}};
        for (const auto& override : overrides) {
            config[override.first] = override.second;
        }
        return config;
    }

    void TearDown() override
    {
        fs::remove_all(_tempDir);
    }

    fs::path tempDir()
    {
        return _tempDir;
    }

private:
    fs::path _tempDir;
};

namespace fs = std::filesystem;


TEST_F(ApplicationDirectoriesTest, usingCurrentVersionConfigTrueWhenDirMatchesVersion)
{
    // Arrange
    constexpr int major = 3;
    constexpr int minor = 7;
    const fs::path testPath = fs::path("some_kind_of_config")
        / App::ApplicationDirectories::versionStringForPath(major, minor);

    // Act: generate a directory structure with the same version
    auto configuration = generateConfig(
        {{"AppVersionMajor", std::to_string(major)}, {"AppVersionMinor", std::to_string(minor)}});
    auto appDirs = std::make_shared<App::ApplicationDirectories>(configuration);

    // Assert
    EXPECT_TRUE(appDirs->usingCurrentVersionConfig(testPath));
}
