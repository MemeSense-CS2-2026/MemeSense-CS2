; =========================
; AutoHotkey v1 script (.ahk)
; Горячие клавиши:
;  Win + D  -> вставить дату (YYYY-MM-DD)
;  Win + T  -> вставить время (HH:MM)
;  Win + S  -> поиск в Google выделенного текста
;  Win + Q  -> закрыть активное окно (Alt+F4)
;  Win + A  -> сделать окно "поверх всех" (toggle)
;  Win + ←/→ -> привязка окна к левому/правому краю
;  Win + Esc -> выход из скрипта
; =========================

#NoEnv
SendMode Input
SetTitleMatchMode, 2

; Win + D -> дата
#d::
FormatTime, CurrentDate,, yyyy-MM-dd
SendInput, %CurrentDate%
return

; Win + T -> время
#t::
FormatTime, CurrentTime,, HH:mm
SendInput, %CurrentTime%
return

; Win + Q -> закрыть активное окно
#q::
SendInput, !{F4}
return

; Win + A -> Always on top (toggle)
#a::
WinGet, ExStyle, ExStyle, A
if (ExStyle & 0x8) {
    Winset, AlwaysOnTop, Off, A
    TrayTip, AutoHotkey, AlwaysOnTop: OFF, 1
} else {
    Winset, AlwaysOnTop, On, A
    TrayTip, AutoHotkey, AlwaysOnTop: ON, 1
}
return

; Win + S -> поиск выделенного текста
#s::
ClipSaved := ClipboardAll
Clipboard := ""
SendInput, ^c
ClipWait, 0.4
q := Clipboard
Clipboard := ClipSaved
if (q = "") {
    TrayTip, AutoHotkey, Ничего не выделено., 1
    return
}
q := StrReplace(q, "`r", " ")
q := StrReplace(q, "`n", " ")
q := StrReplace(q, "#", "%23") ; минимальное экранирование
Run, https://www.google.com/search?q=%q%
return

; Win + Left / Right -> "приклеить" окно к краю экрана
#Left::
WinGetPos, X, Y, W, H, A
SysGet, SW, 78  ; Screen width
SysGet, SH, 79  ; Screen height
WinMove, A,, 0, 0, SW/2, SH
return

#Right::
SysGet, SW, 78
SysGet, SH, 79
WinMove, A,, SW/2, 0, SW/2, SH
return

; Win + Esc -> выход
#Esc::
ExitApp
return