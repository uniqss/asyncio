@set trunk_dir=%cd%
pushd %trunk_dir%

md %trunk_dir%\BuildDebug
cd %trunk_dir%\BuildDebug
cmake -DCMAKE_BUILD_TYPE=Debug %trunk_dir%
cmake --build ./

popd
pause
