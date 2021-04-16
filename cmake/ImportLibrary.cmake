
# 定义一个项目，下载并解压指定的外部依赖库压缩包
# 首先将模板 cmake/DownloadLibrary.cmake.in 实例化为文件 ${pjname}-download/CMakeLists.txt，
# 模板定义了下载过程，必须在构建时才下载。因此，随后构建这个 CMakeLists.txt 定义的项目将
# 依赖库加载并解压。
# 
# 参数
#   pjname: 依赖库名称
#   pjurl: 依赖库下载地址
#   pjmd5: 要下载的依赖库文件的 md5
# 变量输出：
#   IMPORT_SRC 依赖库代码解压后的路径
#   IMPORT_BUILD 依赖库代码编译路径
macro(import_library pjname pjurl pjmd5)
  set(DOWNLOAD_PROJECT_NAME ${pjname})
  set(DOWNLOAD_PROJECT_URL ${pjurl})
  set(DOWNLOAD_PROJECT_MD5 ${pjmd5})

  configure_file("${PROJECT_SOURCE_DIR}/cmake/DownloadLibrary.cmake.in"
                 "${pjname}-download/CMakeLists.txt" @ONLY)

  execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${pjname}-download)
  if(result)
    message(FATAL_ERROR "Prepare cmake step for ${pjname} failed: ${result}")
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${pjname}-download)
  if(result)
    message(FATAL_ERROR "Download step for ${pjname} failed: ${result}")
  endif()

  set(IMPORT_SRC ${CMAKE_CURRENT_BINARY_DIR}/${pjname}-download/${pjname}-src)
  set(IMPORT_BUILD
      ${CMAKE_CURRENT_BINARY_DIR}/${pjname}-download/${pjname}-build)
endmacro()
