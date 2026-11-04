gci $PSScriptRoot -r test_*.cpp | % { "--------------`n$($_.Name)"; g++ -std=c++17 -Wall -Wextra $_ -o t.exe; if ($?) { ./t.exe }; rm t.exe -ea 0; "--------------`n" }
