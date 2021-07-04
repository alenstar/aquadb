// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <limits.h>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "common.h"
#include "commonfuncs.h"
#include "commontypes.h"
#include "filestream.h"
#include "fileutil.h"
#include "logdef.h"

#ifdef _WIN32
#include <Shlwapi.h>
#include <commdlg.h> // for GetSaveFileName
#include <direct.h>  // getcwd
#include <io.h>
#include <objbase.h> // guid stuff
#include <shellapi.h>
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>
#include <sys/param.h>
#endif

#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif

#undef SPDLOG_TAG
#define SPDLOG_TAG "util"
#define COMMON SPDLOG_TAG

#define DIR_SEP "/"
#define DIR_SEP_CHR '/'

// This namespace has various generic functions related to files and paths.
// The code still needs a ton of cleanup.
// REMEMBER: strdup considered harmful!
namespace File {
#ifdef ANDROID
static std::string s_android_sys_directory;
#endif

#ifdef _WIN32
FileInfo::FileInfo(const std::string &path)
{
    m_exists = _tstat64(UTF8ToTStr(path).c_str(), &m_stat) == 0;
}

FileInfo::FileInfo(const char *path) : FileInfo(std::string(path)) {}
#else
FileInfo::FileInfo(const std::string &path) : FileInfo(path.c_str()) {}

FileInfo::FileInfo(const char *path) { m_exists = stat(path, &m_stat) == 0; }
#endif

FileInfo::FileInfo(int fd) { m_exists = fstat(fd, &m_stat); }

bool FileInfo::Exists() const { return m_exists; }

bool FileInfo::IsDirectory() const
{
    return m_exists ? S_ISDIR(m_stat.st_mode) : false;
}

bool FileInfo::IsFile() const
{
    return m_exists ? !S_ISDIR(m_stat.st_mode) : false;
}

u64 FileInfo::GetSize() const { return IsFile() ? m_stat.st_size : 0; }

// Returns true if the path exists
bool Exists(const std::string &path) { return FileInfo(path).Exists(); }

// Returns true if the path exists and is a directory
bool IsDirectory(const std::string &path)
{
#ifdef _WIN32
    return PathIsDirectory(UTF8ToUTF16(path).c_str());
#else
    return FileInfo(path).IsDirectory();
#endif
}

// Returns true if the path exists and is a file
bool IsFile(const std::string &path) { return FileInfo(path).IsFile(); }

// Deletes a given filename, return true on success
// Doesn't supports deleting a directory
bool Delete(const std::string &filename)
{
    LOGI("Delete: file %s", filename.c_str());

    const FileInfo file_info(filename);

    // Return true because we care about the file no
    // being there, not the actual delete.
    if (!file_info.Exists()) {
        LOGW("Delete: %s does not exist", filename.c_str());
        return true;
    }

    // We can't delete a directory
    if (file_info.IsDirectory()) {
        LOGW("Delete failed: %s is a directory", filename.c_str());
        return false;
    }

#ifdef _WIN32
    if (!DeleteFile(UTF8ToTStr(filename).c_str())) {
        LOGW("Delete: DeleteFile failed on %s: %s", filename.c_str(),
             GetLastErrorString().c_str());
        return false;
    }
#else
    if (unlink(filename.c_str()) == -1) {
        LOGW("Delete: unlink failed on %s: %s", filename.c_str(),
             LastStrerrorString().c_str());
        return false;
    }
#endif

    return true;
}

// Returns true if successful, or path already exists.
bool CreateDir(const std::string &path)
{
    LOGI("CreateDir: directory %s", path.c_str());
#ifdef _WIN32
    if (::CreateDirectory(UTF8ToTStr(path).c_str(), nullptr)) return true;
    DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS) {
        LOGW("CreateDir: CreateDirectory failed on %s: already exists",
             path.c_str());
        return true;
    }
    LOGE("CreateDir: CreateDirectory failed on %s: %i", path.c_str(), error);
    return false;
#else
    if (mkdir(path.c_str(), 0755) == 0) return true;

    int err = errno;

    if (err == EEXIST) {
        LOGW("CreateDir: mkdir failed on %s: already exists", path.c_str());
        return true;
    }

    LOGE("CreateDir: mkdir failed on %s: %s", path.c_str(), strerror(err));
    return false;
#endif
}

// Creates the full path of fullPath returns true on success
bool CreateFullPath(const std::string &fullPath)
{
    int panicCounter = 100;
    LOGI("CreateFullPath: path %s", fullPath.c_str());

    if (Exists(fullPath)) {
        LOGI("CreateFullPath: path exists %s", fullPath.c_str());
        return true;
    }

    size_t position = 0;
    while (true) {
        // Find next sub path
        position = fullPath.find(DIR_SEP_CHR, position);

        // we're done, yay!
        if (position == fullPath.npos) return true;

        // Include the '/' so the first call is CreateDir("/") rather than
        // CreateDir("")
        std::string const subPath(fullPath.substr(0, position + 1));
        if (!IsDirectory(subPath)) File::CreateDir(subPath);

        // A safety check
        panicCounter--;
        if (panicCounter <= 0) {
            LOGE("CreateFullPath: directory structure is too deep");
            return false;
        }
        position++;
    }
}

// Deletes a directory filename, returns true on success
bool DeleteDir(const std::string &filename)
{
    LOGI("DeleteDir: directory %s", filename.c_str());

    // check if a directory
    if (!IsDirectory(filename)) {
        LOGE("DeleteDir: Not a directory %s", filename.c_str());
        return false;
    }

#ifdef _WIN32
    if (::RemoveDirectory(UTF8ToTStr(filename).c_str())) return true;
    LOGE("DeleteDir: RemoveDirectory failed on %s: %s", filename.c_str(),
         GetLastErrorString().c_str());
#else
    if (rmdir(filename.c_str()) == 0) return true;
    LOGE("DeleteDir: rmdir failed on %s: %s", filename.c_str(),
         LastStrerrorString().c_str());
#endif

    return false;
}

// renames file srcFilename to destFilename, returns true on success
bool Rename(const std::string &srcFilename, const std::string &destFilename)
{
    LOGI("Rename: %s --> %s", srcFilename.c_str(), destFilename.c_str());
#ifdef _WIN32
    auto sf = UTF8ToTStr(srcFilename);
    auto df = UTF8ToTStr(destFilename);
    // The Internet seems torn about whether ReplaceFile is atomic or not.
    // Hopefully it's atomic enough...
    if (ReplaceFile(df.c_str(), sf.c_str(), nullptr,
                    REPLACEFILE_IGNORE_MERGE_ERRORS, nullptr, nullptr))
        return true;
    // Might have failed because the destination doesn't exist.
    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
        if (MoveFile(sf.c_str(), df.c_str())) return true;
    }
    LOGE("Rename: MoveFile failed on %s --> %s: %s", srcFilename.c_str(),
         destFilename.c_str(), GetLastErrorString().c_str());
#else
    if (rename(srcFilename.c_str(), destFilename.c_str()) == 0) return true;
    LOGE("Rename: rename failed on %s --> %s: %s", srcFilename.c_str(),
         destFilename.c_str(), LastStrerrorString().c_str());
#endif
    return false;
}

#ifndef _WIN32
static void FSyncPath(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd != -1) {
        fsync(fd);
        close(fd);
    }
}
#endif

bool RenameSync(const std::string &srcFilename, const std::string &destFilename)
{
    if (!Rename(srcFilename, destFilename)) return false;
#ifdef _WIN32
    int fd = _topen(UTF8ToTStr(srcFilename).c_str(), _O_RDONLY);
    if (fd != -1) {
        _commit(fd);
        close(fd);
    }
#else
    char *path = strdup(srcFilename.c_str());
    FSyncPath(path);
    FSyncPath(dirname(path));
    free(path);
    path = strdup(destFilename.c_str());
    FSyncPath(dirname(path));
    free(path);
#endif
    return true;
}

// copies file source_path to destination_path, returns true on success
bool Copy(const std::string &source_path, const std::string &destination_path)
{
    LOGI("Copy: %s --> %s", source_path.c_str(), destination_path.c_str());
#ifdef _WIN32
    if (CopyFile(UTF8ToTStr(source_path).c_str(),
                 UTF8ToTStr(destination_path).c_str(), FALSE))
        return true;

    LOGE("Copy: failed %s --> %s: %s", source_path.c_str(),
         destination_path.c_str(), GetLastErrorString().c_str());
    return false;
#else
    std::ifstream source{source_path, std::ios::binary};
    std::ofstream destination{destination_path, std::ios::binary};
    destination << source.rdbuf();
    return source.good() && destination.good();
#endif
}

// Returns the size of a file (or returns 0 if the path isn't a file that
// exists)
u64 GetSize(const std::string &path) { return FileInfo(path).GetSize(); }

// Overloaded GetSize, accepts file descriptor
u64 GetSize(const int fd) { return FileInfo(fd).GetSize(); }

// Overloaded GetSize, accepts FILE*
u64 GetSize(FILE *f)
{
    // can't use off_t here because it can be 32-bit
    u64 pos = ftello(f);
    if (fseeko(f, 0, SEEK_END) != 0) {
        LOGE("GetSize: seek failed %p: %s", f, LastStrerrorString().c_str());
        return 0;
    }

    u64 size = ftello(f);
    if ((size != pos) && (fseeko(f, pos, SEEK_SET) != 0)) {
        LOGE("GetSize: seek failed %p: %s", f, LastStrerrorString().c_str());
        return 0;
    }

    return size;
}

// creates an empty file filename, returns true on success
bool CreateEmptyFile(const std::string &filename)
{
    LOGI("CreateEmptyFile: %s", filename.c_str());

    if (!Streams::FileStream(filename, Streams::Stream::AccessMode::READ_WRITE)
             .good()) {
        LOGE("CreateEmptyFile: failed %s: %s", filename.c_str(),
             LastStrerrorString().c_str());
        return false;
    }

    return true;
}

// Recursive or non-recursive list of files and directories under directory.
FSTEntry ScanDirectoryTree(const std::string &directory, bool recursive)
{
    LOGI("ScanDirectoryTree: directory %s", directory.c_str());
    FSTEntry parent_entry;
    parent_entry.physicalName = directory;
    parent_entry.isDirectory = true;
    parent_entry.size = 0;
#ifdef _WIN32
    // Find the first file in the directory.
    WIN32_FIND_DATA ffd;

    HANDLE hFind = FindFirstFile(UTF8ToTStr(directory + "\\*").c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        return parent_entry;
    }
    // Windows loop
    do {
        const std::string virtual_name(TStrToUTF8(ffd.cFileName));
#else
    DIR *dirp = opendir(directory.c_str());
    if (!dirp) return parent_entry;

    // non Windows loop
    while (dirent *result = readdir(dirp)) {
        const std::string virtual_name(result->d_name);
#endif
        if (virtual_name == "." || virtual_name == "..") continue;
        auto physical_name = directory + DIR_SEP + virtual_name;
        FSTEntry entry;
        const FileInfo file_info(physical_name);
        entry.isDirectory = file_info.IsDirectory();
        if (entry.isDirectory) {
            if (recursive)
                entry = ScanDirectoryTree(physical_name, true);
            else
                entry.size = 0;
            parent_entry.size += entry.size;
        }
        else {
            entry.size = file_info.GetSize();
        }
        entry.virtualName = virtual_name;
        entry.physicalName = physical_name;

        ++parent_entry.size;
        // Push into the tree
        parent_entry.children.push_back(entry);
#ifdef _WIN32
    } while (FindNextFile(hFind, &ffd) != 0);
    FindClose(hFind);
#else
    }
    closedir(dirp);
#endif

    return parent_entry;
}

// Deletes the given directory and anything under it. Returns true on success.
bool DeleteDirRecursively(const std::string &directory)
{
    LOGI("DeleteDirRecursively: %s", directory.c_str());
    bool success = true;

#ifdef _WIN32
    // Find the first file in the directory.
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(UTF8ToTStr(directory + "\\*").c_str(), &ffd);

    if (hFind == INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        return false;
    }

    // Windows loop
    do {
        const std::string virtualName(TStrToUTF8(ffd.cFileName));
#else
    DIR *dirp = opendir(directory.c_str());
    if (!dirp) return false;

    // non Windows loop
    while (dirent *result = readdir(dirp)) {
        const std::string virtualName = result->d_name;
#endif

        // check for "." and ".."
        if (((virtualName[0] == '.') && (virtualName[1] == '\0')) ||
            ((virtualName[0] == '.') && (virtualName[1] == '.') &&
             (virtualName[2] == '\0')))
            continue;

        std::string newPath = directory + DIR_SEP_CHR + virtualName;
        if (IsDirectory(newPath)) {
            if (!DeleteDirRecursively(newPath)) {
                success = false;
                break;
            }
        }
        else {
            if (!File::Delete(newPath)) {
                success = false;
                break;
            }
        }

#ifdef _WIN32
    } while (FindNextFile(hFind, &ffd) != 0);
    FindClose(hFind);
#else
    }
    closedir(dirp);
#endif
    if (success) File::DeleteDir(directory);

    return success;
}

// Create directory and copy contents (does not overwrite existing files)
void CopyDir(const std::string &source_path, const std::string &dest_path,
             bool destructive)
{
    if (source_path == dest_path) return;
    if (!Exists(source_path)) return;
    if (!Exists(dest_path)) File::CreateFullPath(dest_path);

#ifdef _WIN32
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(UTF8ToTStr(source_path + "\\*").c_str(), &ffd);

    if (hFind == INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        return;
    }

    do {
        const std::string virtualName(TStrToUTF8(ffd.cFileName));
#else
    DIR *dirp = opendir(source_path.c_str());
    if (!dirp) return;

    while (dirent *result = readdir(dirp)) {
        const std::string virtualName(result->d_name);
#endif
        // check for "." and ".."
        if (virtualName == "." || virtualName == "..") continue;

        std::string source = source_path + DIR_SEP + virtualName;
        std::string dest = dest_path + DIR_SEP + virtualName;
        if (IsDirectory(source)) {
            if (!Exists(dest)) File::CreateFullPath(dest + DIR_SEP);
            CopyDir(source, dest, destructive);
        }
        else if (!destructive && !Exists(dest)) {
            Copy(source, dest);
        }
        else if (destructive) {
            Rename(source, dest);
        }
#ifdef _WIN32
    } while (FindNextFile(hFind, &ffd) != 0);
    FindClose(hFind);
#else
    }
    closedir(dirp);
#endif
}

// Returns the current directory
std::string GetCurrentDir()
{
    // Get the current working directory (getcwd uses malloc)
    char *dir = __getcwd(nullptr, 0);
    if (!dir) {
        LOGE("GetCurrentDirectory failed: %s", LastStrerrorString().c_str());
        return nullptr;
    }
    std::string strDir = dir;
    free(dir);
    return strDir;
}

// Sets the current directory to the given directory
bool SetCurrentDir(const std::string &directory)
{
    return __chdir(directory.c_str()) == 0;
}

std::string CreateTempDir()
{
#ifdef _WIN32
    TCHAR temp[MAX_PATH];
    if (!GetTempPath(MAX_PATH, temp)) return "";

    GUID guid;
    CoCreateGuid(&guid);
    TCHAR tguid[40];
    StringFromGUID2(guid, tguid, 39);
    tguid[39] = 0;
    std::string dir = TStrToUTF8(temp) + "/" + TStrToUTF8(tguid);
    if (!CreateDir(dir)) return "";
    dir = ReplaceAll(dir, "\\", DIR_SEP);
    return dir;
#else
    const char *base = getenv("TMPDIR") ?: "/tmp";
    std::string path = std::string(base) + "/DolphinWii.XXXXXX";
    if (!mkdtemp(&path[0])) return "";
    return path;
#endif
}

std::string GetTempFilenameForAtomicWrite(const std::string &path)
{
    std::string abs = path;
#ifdef _WIN32
    TCHAR absbuf[MAX_PATH];
    if (_tfullpath(absbuf, UTF8ToTStr(path).c_str(), MAX_PATH) != nullptr)
        abs = TStrToUTF8(absbuf);
#else
    char absbuf[PATH_MAX];
    if (realpath(path.c_str(), absbuf) != nullptr) abs = absbuf;
#endif
    return abs + ".xxx";
}

#if defined(__APPLE__)
std::string GetBundleDirectory()
{
    CFURLRef BundleRef;
    char AppBundlePath[MAXPATHLEN];
    // Get the main bundle for the app
    BundleRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef BundlePath =
        CFURLCopyFileSystemPath(BundleRef, kCFURLPOSIXPathStyle);
    CFStringGetFileSystemRepresentation(BundlePath, AppBundlePath,
                                        sizeof(AppBundlePath));
    CFRelease(BundleRef);
    CFRelease(BundlePath);

    return AppBundlePath;
}
#endif

std::string GetExePath()
{
    static std::string dolphin_path;
    if (dolphin_path.empty()) {
#ifdef _WIN32
        TCHAR dolphin_exe_path[2048];
        TCHAR dolphin_exe_expanded_path[MAX_PATH];
        GetModuleFileName(nullptr, dolphin_exe_path,
                          ARRAYSIZE(dolphin_exe_path));
        if (_tfullpath(dolphin_exe_expanded_path, dolphin_exe_path,
                       ARRAYSIZE(dolphin_exe_expanded_path)) != nullptr)
            dolphin_path = TStrToUTF8(dolphin_exe_expanded_path);
        else
            dolphin_path = TStrToUTF8(dolphin_exe_path);
#else
        char dolphin_exe_path[PATH_MAX];
        ssize_t len = ::readlink("/proc/self/exe", dolphin_exe_path,
                                 sizeof(dolphin_exe_path));
        if (len == -1 || len == sizeof(dolphin_exe_path)) {
            len = 0;
        }
        dolphin_exe_path[len] = '\0';
        dolphin_path = dolphin_exe_path;
#endif
    }
    return dolphin_path;
}

std::string GetExeDirectory()
{
    std::string exe_path = GetExePath();
#ifdef _WIN32
    return exe_path.substr(0, exe_path.rfind('\\'));
#else
    return exe_path.substr(0, exe_path.rfind('/'));
#endif
}

bool WriteStringToFile(const std::string &str, const std::string &filename)
{
    return Streams::FileStream(filename,
                               Streams::Stream::AccessMode::READ_WRITE)
        .write(str.data(), str.size());
}

bool ReadFileToString(const std::string &filename, std::string &str)
{
    Streams::FileStream file(filename, Streams::Stream::AccessMode::READ_WRITE);

    if (!file.good()) return false;

    str.resize(file.size());
    return file.read(&str[0], str.size());
}

} // namespace File
