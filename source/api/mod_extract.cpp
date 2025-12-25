#include "api/mod_extract.hpp"
#include "smd_utils/progress_event.hpp"

#include <archive.h>
#include <archive_entry.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <switch.h>
#include <thread>
#include <borealis.hpp>

const std::string smash_tid = "01006A800016E000";

// Use a distinct namespace to avoid clashing with the app's existing extract utilities
namespace mod_extract {
    int getFileCount(const std::string& archivePath) {
        struct archive* archive;
        struct archive_entry* entry;
        int fileCount = 0;

        archive = archive_read_new();
        archive_read_support_format_all(archive);
        archive_read_support_filter_all(archive);

        if(archive_read_open_filename(archive, archivePath.c_str(), 10240) == ARCHIVE_OK) {
            while(archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
                fileCount++;
            }
            archive_read_close(archive);
        }

        archive_read_free(archive);
        return fileCount;
    }

    s64 getTotalArchiveSize(const std::string& archivePath) {
        struct archive* archive;
        struct archive_entry* entry;
        s64 totalSize = 0;

        archive = archive_read_new();
        archive_read_support_format_all(archive);
        archive_read_support_filter_all(archive);

        if(archive_read_open_filename(archive, archivePath.c_str(), 10240) == ARCHIVE_OK) {
            while(archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
                totalSize += archive_entry_size(entry);
            }
            archive_read_close(archive);
        }

        archive_read_free(archive);
        return totalSize;
    }

bool extractEntry(const std::string& archiveFile, const std::string& outputDir, const std::string& tid) {
        chdir("sdmc:/");
        struct archive* archive = archive_read_new();

        brls::Logger::debug("Extracting {} to {}", archiveFile, outputDir);

        // Create output directory if it doesn't exist
        std::filesystem::create_directories(outputDir);

        archive_read_support_format_all(archive);
        int result = archive_read_open_filename(archive, archiveFile.c_str(), 10240);
        if (result != ARCHIVE_OK) {
            brls::Logger::error("Failed to open archive: {}", archiveFile);
            archive_read_free(archive);
            //std::filesystem::remove(archiveFile);
            ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
            return false;
        }
        struct archive_entry* entry;
        ProgressEvent::instance().setTotalSteps(getFileCount(archiveFile));
        ProgressEvent::instance().setStep(0);

        s64 freeStorage;
        if(R_SUCCEEDED(nsGetFreeSpaceSize(NcmStorageId_SdCard, &freeStorage)) && getTotalArchiveSize(archiveFile) * 1.1 > freeStorage) {
            brls::Logger::error("sd is full");
            archive_read_free(archive);
            std::filesystem::remove(archiveFile);
            ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
            brls::Application::crash("full");
            std::this_thread::sleep_for(std::chrono::microseconds(2000000));
            brls::Application::quit();
            return false;
        }

        while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
            if (ProgressEvent::instance().getInterupt()) {
                archive_read_close(archive);
                archive_read_free(archive);
                std::filesystem::remove(archiveFile);
                ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
                return false;
            }
            const char* entryName = archive_entry_pathname(entry);
            
            if ((tid != smash_tid)) {
                if (std::string(entryName).find("romfs/") != std::string::npos || std::string(entryName).find("exefs/") != std::string::npos || std::string(entryName).find("exefs_patches/") != std::string::npos) {
                    
                    std::string outputFilePath;


                    if (std::string(entryName).find("romfs/") != std::string::npos) //romfs
                        outputFilePath = fmt::format("{}/contents/{}/{}", outputDir, tid, std::string(entryName).substr(std::string(entryName).find("romfs/")));
                    else if (std::string(entryName).find("exefs_patches/") != std::string::npos)//exefs_patches
                        outputFilePath = fmt::format("{}/{}", outputDir, std::string(entryName).substr(std::string(entryName).find("exefs_patches/")));
                    else //Exefs
                        outputFilePath = fmt::format("{}/contents/{}/{}", outputDir, tid, std::string(entryName).substr(std::string(entryName).find("exefs/")));

                    if (std::string(entryName).find("|") != std::string::npos)
                        outputFilePath = outputFilePath.substr(0, outputFilePath.find("|"));
                  
                    brls::Logger::debug("Extracting file {} to {}", entryName,outputFilePath);
                    std::filesystem::path outputPath(outputFilePath);
                    std::filesystem::create_directories(outputPath.parent_path());

                    if (archive_entry_filetype(entry) == AE_IFDIR) {
                        ProgressEvent::instance().incrementStep(1);
                        // Skip directories
                        continue;
                    }


                    std::ofstream outputFile(outputFilePath, std::ios::binary);
                    if (!outputFile) {
                        brls::Logger::error("Failed to create output file: {}", outputFilePath);
                        archive_read_free(archive);
                        std::filesystem::remove(archiveFile);
                        ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
                        return false;
                    }

                    const size_t bufferSize = 100000;
                    char buffer[bufferSize];
                    ssize_t bytesRead;
                    while ((bytesRead = archive_read_data(archive, buffer, bufferSize)) > 0) {
                        outputFile.write(buffer, bytesRead);
                    }

                    outputFile.close();

                    ProgressEvent::instance().incrementStep(1);
                } else {
                    brls::Logger::debug("Skipping {}", entryName);
                }
            } else {
                // Smash bros mods
                std::string outputFilePath = fmt::format("sdmc:/ultimate/mods/{}",std::string(entryName));
                std::filesystem::path outputPath(outputFilePath);
                std::filesystem::create_directories(outputPath.parent_path());
                if (archive_entry_filetype(entry) == AE_IFDIR) {
                    // Create the directory
                    if (!std::filesystem::create_directory(outputPath)) {
                        brls::Logger::error("Failed to create directory: {}", outputFilePath);
                    }
                    ProgressEvent::instance().incrementStep(1);
                    continue;
                }

                std::ofstream outputFile(outputFilePath, std::ios::binary);
                if (!outputFile) {
                    brls::Logger::error("Failed to create output file: {}", outputFilePath);
                    archive_read_free(archive);
                    std::filesystem::remove(archiveFile);
                    ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
                    return false;
                }

                const size_t bufferSize = 100000;
                char buffer[bufferSize];
                ssize_t bytesRead;
                while ((bytesRead = archive_read_data(archive, buffer, bufferSize)) > 0) {
                    outputFile.write(buffer, bytesRead);
                }

                outputFile.close();

                brls::Logger::debug("Extracted file: {}", outputFilePath);
                ProgressEvent::instance().incrementStep(1);
            }
        }

        archive_read_close(archive);
        archive_read_free(archive);
        std::filesystem::remove(archiveFile);
        ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
        return true;
    }

    // Helper function to check if file is an archive
    bool isArchiveFile(const std::string& filename) {
        std::string lower = filename;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower.ends_with(".zip") || lower.ends_with(".rar") || 
               lower.ends_with(".7z") || lower.ends_with(".bnp") ||
               lower.ends_with(".gz") || lower.ends_with(".tar");
    }

    // Simple extraction without Atmosphere path structure (for /mods folder)
    bool extractSimple(const std::string& archiveFile, const std::string& outputDir, bool updateProgress = true, int* currentStep = nullptr) {
        chdir("sdmc:/");
        struct archive* archive = archive_read_new();

        brls::Logger::debug("Extracting simple {} to {}", archiveFile, outputDir);

        std::filesystem::create_directories(outputDir);

        archive_read_support_format_all(archive);
        archive_read_support_filter_all(archive);

        int result = archive_read_open_filename(archive, archiveFile.c_str(), 10240);
        if (result != ARCHIVE_OK) {
            brls::Logger::error("Failed to open archive: {} (error: {})", archiveFile, archive_error_string(archive));
            brls::Logger::error("Archive result code: {}", result);
            archive_read_free(archive);
            return false;
        }

        // Set up progress tracking only if updateProgress is true
        int localStep = 0;
        int fileCount = 0;
        if (updateProgress) {
            fileCount = getFileCount(archiveFile);
            ProgressEvent::instance().setTotalSteps(fileCount);
            ProgressEvent::instance().setStep(0);
        }

        struct archive_entry* entry;
        while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
            if (ProgressEvent::instance().getInterupt()) {
                archive_read_close(archive);
                archive_read_free(archive);
                ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
                return false;
            }
            const char* entryName = archive_entry_pathname(entry);
            std::string outputFilePath = fmt::format("{}/{}", outputDir, entryName);

            brls::Logger::debug("Extracting file {} to {}", entryName, outputFilePath);
            std::filesystem::path outputPath(outputFilePath);
            std::filesystem::create_directories(outputPath.parent_path());

            if (archive_entry_filetype(entry) == AE_IFDIR) {
                std::filesystem::create_directories(outputPath);
                // Update progress even for directories
                if (updateProgress) {
                    localStep++;
                    ProgressEvent::instance().setStep(localStep);
                } else if (currentStep) {
                    (*currentStep)++;
                    ProgressEvent::instance().setStep(*currentStep);
                }
                continue;
            }

            std::ofstream outputFile(outputFilePath, std::ios::binary);
            if (!outputFile) {
                brls::Logger::error("Failed to create output file: {}", outputFilePath);
                archive_read_close(archive);
                archive_read_free(archive);
                return false;
            }

            const size_t bufferSize = 100000;
            char buffer[bufferSize];
            ssize_t bytesRead;
            while ((bytesRead = archive_read_data(archive, buffer, bufferSize)) > 0) {
                outputFile.write(buffer, bytesRead);
            }

            outputFile.close();
            
            // Update progress
            if (updateProgress) {
                localStep++;
                ProgressEvent::instance().setStep(localStep);
            } else if (currentStep) {
                (*currentStep)++;
                ProgressEvent::instance().setStep(*currentStep);
            }
        }

        archive_read_close(archive);
        archive_read_free(archive);
        if (updateProgress) {
            ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
        }
        return true;
    }

    // Recursively count files in archive including nested archives
    int getTotalFileCountRecursive(const std::string& archivePath, const std::string& tempDir) {
        int totalCount = getFileCount(archivePath);
        
        // Extract to temp location to scan for nested archives
        std::string scanDir = fmt::format("{}/scan_temp", tempDir);
        std::filesystem::create_directories(scanDir);
        
        // Do a quick extraction to temp just to scan
        if (extractSimple(archivePath, scanDir, false, nullptr)) {
            // Scan for nested archives
            for (const auto& entry : std::filesystem::recursive_directory_iterator(scanDir)) {
                if (entry.is_regular_file() && isArchiveFile(entry.path().string())) {
                    totalCount += getFileCount(entry.path().string());
                }
            }
        }
        
        // Clean up temp directory
        std::filesystem::remove_all(scanDir);
        return totalCount;
    }

    // Recursive extraction: extract archive and any nested archives found
    bool extractRecursive(const std::string& archivePath, const std::string& outputDir) {
        // Quick scan: check if archive contains nested archives
        bool hasNestedArchives = false;
        struct archive* archive = archive_read_new();
        archive_read_support_format_all(archive);
        archive_read_support_filter_all(archive);

        if (archive_read_open_filename(archive, archivePath.c_str(), 10240) == ARCHIVE_OK) {
            struct archive_entry* entry;
            while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
                const char* entryName = archive_entry_pathname(entry);
                if (isArchiveFile(std::string(entryName))) {
                    hasNestedArchives = true;
                    break;
                }
            }
            archive_read_close(archive);
        }
        archive_read_free(archive);

        // If no nested archives, just do a simple extraction
        if (!hasNestedArchives) {
            bool result = extractSimple(archivePath, outputDir, true, nullptr);
            std::filesystem::remove(archivePath);
            return result;
        }

        // Pre-calculate total file count including all nested archives
        int totalFiles = getTotalFileCountRecursive(archivePath, outputDir);
        ProgressEvent::instance().setTotalSteps(totalFiles);
        ProgressEvent::instance().setStep(0);
        int currentStep = 0;

        // Extract the main archive without internal progress tracking
        if (!extractSimple(archivePath, outputDir, false, &currentStep)) {
            return false;
        }

        // Delete the original archive after extraction
        std::filesystem::remove(archivePath);

        // Now scan the output directory for nested archives
        std::vector<std::string> nestedArchives;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(outputDir)) {
            if (entry.is_regular_file() && isArchiveFile(entry.path().string())) {
                nestedArchives.push_back(entry.path().string());
            }
        }

        // Extract each nested archive recursively
        for (const auto& nestedArchive : nestedArchives) {
            // Extract in the same directory as the nested archive
            std::string nestedOutputDir = std::filesystem::path(nestedArchive).parent_path().string();

            if (!extractSimple(nestedArchive, nestedOutputDir, false, &currentStep)) {
                continue;
            }

            // Delete the nested archive after extraction
            std::filesystem::remove(nestedArchive);

            // Recursively check for more nested archives in the extracted content
            for (const auto& entry : std::filesystem::recursive_directory_iterator(nestedOutputDir)) {
                if (entry.is_regular_file() && isArchiveFile(entry.path().string())) {
                    std::string deepNestedArchive = entry.path().string();

                    // Update total steps for deep nested archive
                    int deepFiles = getFileCount(deepNestedArchive);
                    totalFiles += deepFiles;
                    ProgressEvent::instance().setTotalSteps(totalFiles);

                    std::string deepOutputDir = std::filesystem::path(deepNestedArchive).parent_path().string();
                    if (extractSimple(deepNestedArchive, deepOutputDir, false, &currentStep)) {
                        std::filesystem::remove(deepNestedArchive);
                    }
                }
            }
        }

        ProgressEvent::instance().setStep(ProgressEvent::instance().getMax());
        return true;
    }

    // Helper function to check if a folder name is a 16-character Title ID
    bool isTitleIdFolder(const std::string& folderName) {
        if (folderName.length() != 16) return false;
        for (char c : folderName) {
            if (!std::isxdigit(c)) return false;
        }
        return true;
    }

    // Fix mod structure if Title ID folder is missing
    void fixModStructure(const std::string& modDir, const std::string& titleId) {
        brls::Logger::debug("Checking mod structure in: {}", modDir);

        // Check if a 16-character Title ID folder already exists
        bool hasTitleIdFolder = false;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(modDir)) {
                if (entry.is_directory()) {
                    std::string folderName = entry.path().filename().string();
                    if (isTitleIdFolder(folderName)) {
                        hasTitleIdFolder = true;
                        brls::Logger::debug("Found existing Title ID folder: {}", folderName);
                        break;
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            brls::Logger::error("Error checking for Title ID folder: {}", e.what());
            return;
        }

        if (hasTitleIdFolder) {
            brls::Logger::debug("Title ID folder exists, no restructuring needed");
            return;
        }

        brls::Logger::info("No Title ID folder found, restructuring mod with Title ID: {}", titleId);

        // Create Title ID folder
        std::string titleIdPath = fmt::format("{}/{}", modDir, titleId);
        std::filesystem::create_directories(titleIdPath);

        // Search for romfs and exefs folders recursively
        std::string romfsPath, exefsPath;

        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(modDir)) {
                if (entry.is_directory()) {
                    std::string folderName = entry.path().filename().string();

                    if (folderName == "romfs" && romfsPath.empty()) {
                        romfsPath = entry.path().string();
                        brls::Logger::debug("Found romfs at: {}", romfsPath);
                    }
                    else if (folderName == "exefs" && exefsPath.empty()) {
                        exefsPath = entry.path().string();
                        brls::Logger::debug("Found exefs at: {}", exefsPath);
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            brls::Logger::error("Error searching for romfs/exefs: {}", e.what());
        }

        // Move romfs if found
        if (!romfsPath.empty()) {
            std::string destRomfs = fmt::format("{}/romfs", titleIdPath);
            try {
                std::filesystem::rename(romfsPath, destRomfs);
                brls::Logger::info("Moved romfs to: {}", destRomfs);
            } catch (const std::filesystem::filesystem_error& e) {
                brls::Logger::error("Failed to move romfs: {}", e.what());
            }
        }

        // Move exefs if found
        if (!exefsPath.empty()) {
            std::string destExefs = fmt::format("{}/exefs", titleIdPath);
            try {
                std::filesystem::rename(exefsPath, destExefs);
                brls::Logger::info("Moved exefs to: {}", destExefs);
            } catch (const std::filesystem::filesystem_error& e) {
                brls::Logger::error("Failed to move exefs: {}", e.what());
            }
        }

        // Delete everything else that's not the Title ID folder
        try {
            for (const auto& entry : std::filesystem::directory_iterator(modDir)) {
                std::string entryName = entry.path().filename().string();
                if (entryName != titleId) {
                    std::filesystem::remove_all(entry.path());
                    brls::Logger::debug("Removed: {}", entry.path().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            brls::Logger::error("Error cleaning up extra files: {}", e.what());
        }

        brls::Logger::info("Mod structure fixed successfully");
    }
}