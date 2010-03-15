NAME=notepad

$(NAME).exe : $(NAME).obj $(NAME).res
        link /SUBSYSTEM:WINDOWS $(NAME).obj $(NAME).res
$(NAME).res : $(NAME).rc
        rc $(NAME).rc
$(NAME).obj : $(NAME).asm
        ml /c /coff /Cp $(NAME).asm
