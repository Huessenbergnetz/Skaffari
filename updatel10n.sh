#!/bin/bash

for TMPL in default;
do

    if [ -d "templates/${TMPL}/l10ntmp" ]; then
        rm -rf "templates/${TMPL}/l10ntmp"
    fi
    
    mkdir "templates/${TMPL}/l10ntmp"
    
    if [ ! -d "templates/${TMPL}/l10n" ]; then
        mkdir "templates/${TMPL}/l10n"
    fi

    for FILE in `find templates/${TMPL}/site -name "*.html"`;
    do
        FN=`echo "$FILE" | cut -d'.' -f1`
        FN=`echo $FN | sed 's!/!_!g'`
        python templates/scripts/extract_strings_linguist.py "${FILE}" > "templates/${TMPL}/l10ntmp/${FN}.cpp"
    done
    
    lupdate-qt5 -no-obsolete \
                -source-language en \
                -target-language en \
                -locations none \
                "templates/${TMPL}/l10ntmp" \
                -ts "templates/${TMPL}/l10n/tmpl_${TMPL}.ts"
                
    rm -rf "templates/${TMPL}/l10ntmp"
done

if [ ! -d l10n ]; then
    mkdir l10n
fi

lupdate-qt5 -no-obsolete \
            -source-language en \
            -target-language en \
            -locations none \
            cmd \
            -ts "l10n/skaffaricmd.ts"
            
lupdate-qt5 -no-obsolete \
            -source-language en \
            -target-language en \
            -locations none \
            src \
            -ts "l10n/skaffari.ts"
