#! /bin/bash

grn="\e[0;92m"
red="\e[31m"
rst="\e[0m"

tst=$(curl --no-progress-meter -i -H "Accept-Language:en;q=0.5" -X GET http://localhost:8080/index)
echo 'curl --no-progress-meter -i -H "Accept-Language:en;q=0.5" -X GET http://localhost:8080/index'
if [[ $tst == *"Content-language: en"* ]]; then 
    echo -e "${grn}This test is good${rst}\n\n"
else
    echo -e "${red}False\n${rst}${tst}\n${red}Content-language should be 'en'${rst}\n\n"
fi

tst=$(curl --no-progress-meter -i -H "Accept-Language:fr;q=0.8,en;q=0.5,*;q=0.1"-X GET http://localhost:8080/index)
echo 'curl -i -H "Accept-Language:fr;q=0.8,en;q=0.5,*;q=0.1"-X GET http://localhost:8080/index'
if [[ $tst == *"Content-language: fr"* ]]; then 
    echo -e "${grn}This test is good${rst}\n\n"
else
    echo -e "${red}False\n${rst}${tst}\n${red}Content-language should be 'fr'${rst}\n\n"
fi

tst=$(curl --no-progress-meter -i -H "Accept-Language:fr;q=0.8,en;q=0.2" -H "Accept-Charset:Ux;q=0.9,F8;q=0.1" -X GET http://localhost:8080/ctnNego/index.html)
echo 'curl --no-progress-meter -i -H "Accept-Language:fr;q=0.8,en;q=0.2" -H "Accept-Charset:Ux;q=0.9,F8;q=0.1" -X GET http://localhost:8080/ctnNego/index.html'
if [[ $tst == *"Content-language: fr"* && $tst == *"Content-charset: Ux"* ]]; then 
    echo -e "${grn}This test is good${rst}\n\n"
else
    echo -e "${red}False\n${rst}${tst}\n${red}Content-language should be 'en' and Content-language should be "Ux"${rst}\n\n"
fi

tst=$(curl --no-progress-meter -i -H "Accept-Charset:Ux;q=0.1,F8;q=0.5" -X GET http://localhost:8080/ctnNego/index.html)
echo 'curl --no-progress-meter -i -H "Accept-Charset:Ux;q=0.1,F8;q=0.5" -X GET http://localhost:8080/ctnNego/index.html'
if [[ $tst == *"Content-charset: F8"* ]]; then 
    echo -e "${grn}This test is good${rst}\n\n"
else
    echo -e "${red}False\n${rst}${tst}\n${red}Content-charset should be "F8"${rst}\n\n"
fi

tst=$(curl --no-progress-meter -i -H "Accept-Language:fr;q=0.9,*;q=0.1" -H "Accept-Charset:Ux;q=0.4,F8;q=0.5" -X GET http://localhost:8080/ctnNego/)
echo 'curl --no-progress-meter -i -H "Accept-Language:fr;q=0.9,*;q=0.1" -H "Accept-Charset:Ux;q=0.4,F8;q=0.5" -X GET http://localhost:8080/ctnNego/'
if [[ $tst == *"Content-language: fr"* && $tst == *"Content-charset: F8"* ]]; then 
    echo -e "${grn}This test is good${rst}\n\n"
else
    echo -e "${red}False\n${rst}${tst}\n${red}Content-language should be fr and Content-charset should be "F8"${rst}\n\n"
fi

tst=$(curl --no-progress-meter -i -H "Accept-Language:fr;q=0.9,*;q=0.1" -H "Accept-Charset:Ux;q=0.6,F8;q=0.5" -X GET http://localhost:8080/ctnNego/)
echo 'curl --no-progress-meter -i -H "Accept-Language:fr;q=0.9,*;q=0.1" -H "Accept-Charset:Ux;q=0.6,F8;q=0.5" -X GET http://localhost:8080/ctnNego/'
if [[ $tst == *"Content-language: fr"* && $tst == *"Content-charset: Ux"* ]]; then 
    echo -e "${grn}This test is good${rst}\n\n"
else
    echo -e "${red}False\n${rst}${tst}\n${red}Content-language should be fr and Content-charset should be "Ux"${rst}\n\n"
fi

