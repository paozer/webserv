#! /bin/bash

grn="\e[0;92m"
red="\e[31m"
rst="\e[0m"

tst=$(curl -s -i -H "Accept-Language:en;q=0.5" -X GET http://localhost:8080/nego/index.html)
printf 'curl -s -i -H "Accept-Language:en;q=0.5" -X GET http://localhost:8080/nego/index.html\n'
if [[ $tst == *"Content-language: en"* ]]; then
    printf "${grn}This test is good${rst}\n\n"
else
    printf "${red}False\n${rst}${tst}\n${red}Content-language should be 'en'${rst}\n\n"
fi

tst=$(curl -s -i -H "Accept-Language:fr;q=0.8,en;q=0.5,*;q=0.1"-X GET http://localhost:8080/nego/index.html)
printf 'curl -i -H "Accept-Language:fr;q=0.8,en;q=0.5,*;q=0.1"-X GET http://localhost:8080/nego/index.html\n'
if [[ $tst == *"Content-language: fr"* ]]; then
    printf "${grn}This test is good${rst}\n\n"
else
    printf "${red}False\n${rst}${tst}\n${red}Content-language should be 'fr'${rst}\n\n"
fi

tst=$(curl -s -i -H "Accept-Language:fr;q=0.8,en;q=0.2" -H "Accept-Charset:Ux;q=0.9,F8;q=0.1" -X GET http://localhost:8080/nego/index.html)
printf 'curl -s -i -H "Accept-Language:fr;q=0.8,en;q=0.2" -H "Accept-Charset:Ux;q=0.9,F8;q=0.1" -X GET http://localhost:8080/ctnNego/index.html\n'
if [[ $tst == *"Content-language: en"* && $tst == *"Content-charset: Ux"* ]]; then
    printf "${grn}This test is good${rst}\n\n"
else
    printf "${red}False\n${rst}${tst}\n${red}Content-language should be 'en' and Content-language should be "Ux"${rst}\n\n"
fi

tst=$(curl -s -i -H "Accept-Charset:Ux;q=0.1,F8;q=0.5" -X GET http://localhost:8080/nego/index.html)
printf 'curl -s -i -H "Accept-Charset:Ux;q=0.1,F8;q=0.5" -X GET http://localhost:8080/nego/index.html\n'
if [[ $tst == *"Content-charset: F8"* ]]; then
    printf "${grn}This test is good${rst}\n\n"
else
    printf "${red}False\n${rst}${tst}\n${red}Content-charset should be "F8"${rst}\n\n"
fi
