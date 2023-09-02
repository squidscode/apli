; hangman-based anagram game:
;    a random word is chosen from a list and scrambled,
;        and the scrambled word is displayed to
;        the player along with a current score
;    for a word of length N the score is 2^N
;    the player then repeatedly tries to guess
;        which of the scrambled letters is the
;        first letter in the target word
;    once the player gets the first letter correct,
;        they move on to the next letter
;    on each incorrect guess the player's score is
;        cut in half, after N+1 incorrect guesses
;        the player loses
;    the current list contains words of 5-7 characters,
;        so starting scores are 32, 64, or 128
;
; the player can continue playing with new words,
;    and the game tracks both their total, average,
;    and best scores
;
; *** assumes the list of words is provided in a file
; *** named dictionary.cl, with the actual list of words
; *** stored in a global variable named dictionary
(load "dictionary.cl")


; debugMode set to true iff we want debugging info printed
(defvar debugMode nil)


; (clearscrn)
; -----------
; clears the screen by printing an ascii control sequence
(defconstant Escape (code-char 27))
(defun clearscrn ()
   (format t "~A[2J~A[H" Escape Escape))


; (welcome)
; ---------
; display the rules of the game
(defun welcome ()
   (clearscrn)
   (format t "~%Welcome to hangagram - an anagram solver originally inspired by hangman~%")
   (format t "~%The computer will choose a random word and show you its letters")
   (format t " in scrambled order~%")
   (format t "~%It will then ask you to identify the characters in the word, ")
   (format t "one at a time, in order~%")
   (format t "~%Your score is based on the length of the word, and is decreased ")
   (format t "for each incorrect guess~%")
   (format t "~%Good luck!!~%"))


; (shuffle L)
; -----------
; randomly shuffle the contents of list L
(defun shuffle (L)
   (if (not (listp L)) L
       (block 'gotAList
          (loop for i from (length L) downto 2
             do (rotatef (elt L (random i)) (elt L (1- i))))
          L)))


; (playAgain N)
; -------------
; if this isn't the first game played N is 0 so don't ask,
; otherwise see if the user wishes to play another game
; return true if they want to play again, false otherwise
(defun playAgain (numGames)
   (if (< numGames 1) t
       (block 'askAndCheck
          (format t "Do you wish to play again?~%")
          (format t "Enter Y for yes, anything else for no~%")
          (string= (read) "Y"))))


; (getGuessPos sofar remaining)
; -----------------------------
; given the word they have so far, and the list of remaining characters,
;    get the user to pick one of the remaining characters (by position)
;    that they think is the next character in the target word
(defun getGuessPos (sofar remaining)
   (let ((guessPos nil))
   (if (null sofar)
       (format t "   Pick the first letter in the word by its position in this list:~%   ~A   (positions start at 0)~%   " remaining)
       (format t "   So far you have ~A, pick the next letter (by position) from~%   ~A~%   "
           sofar remaining))
    (setf guessPos (read))
    (if (and (integerp guessPos) (<= 0 guessPos) (< guessPos (length remaining)))
        guessPos ; guess was valid
        (block 'invalidGuess
           (format t "Invalid guess: ~A, please try again~%" guessPos)
           (getGuessPos sofar remaining)))))


; (playRound N)
; -------------
; play the Nth round of the game and return the score
;    (N is 1 for the first game)
(defun playRound (numGames)
   (let
      ((targetWord nil) (targetList nil) ; the whole target word
         (sofarWord nil) (sofarList nil) ; the part of the target they have right so far
         (togoWord nil) (togoList nil)   ; what they still have to get in the target word
         (mixedWord nil) (mixedList nil) ; the letters they still have left over
         (score 0) (numWords 0) (numGuesses 0))
   (labels
      ((debugPrt (&optional (gpos nil) (gchar nil))
          (when debugMode
          (if (not (null gpos)) (format t "Guessed position ~A, " gpos))
          (if (not (null gchar)) (format t "guess char ~A, " gchar))
          (format t "Looking for ~A, got ~A and have ~A left " targetWord sofarWord mixedWord)
          (format t "after ~A guesses, score so far ~A~%~%" 
             numGuesses score)))
       (resultsPrt ()
          (if (= score 0)
              (block 'failed
                 (format t "~%Fail!  The target word was ~A, " targetWord)
                 (format t "you got ~A, with ~A left after ~A guesses, score 0~%~%"
                     sofarWord mixedWord numGuesses))
              (block 'succeeded
                 (format t "~%You got ~A in ~A guesses, score ~A~%~%"
                     targetWord numGuesses score)))))
 

   ; start the round
   (format t "~%********** Playing round ~A **********~%~%" numGames)

   ; select the target word and build a list out of its letters
   (setf numWords (length dictionary))
   (setf targetWord (symbol-name (elt dictionary (random numWords))))
   (setf targetList (coerce targetWord 'list))
   (setf togoWord targetWord)
   (setf togoList (copy-list targetList))

   ; create the scrambled list and build a string out of its letters
   (setf mixedList (shuffle targetList))
   (setf mixedWord (format nil "~{~A~}" mixedList))

   ; work out the starting score for this round
   (setf score (expt 2 (length targetList)))

   ; ------------- actual game play needs to go here ------------
   ; debugging check (during development)
   (debugPrt)
   (do   ( ; locals:
           ;    current user guess by position (in remaining list of letters)
           ;    and the letter that corresponds to
           (guessPos 0) (guessChar nil))

         ( ; quit if score is 0 or mixedList is nil,
           ;    printing the final results
           ;    and returning the score
           (or (< score 1) (null mixedList))
           (resultsPrt)
           score)

        (block 'processGuess
           ; increment the number of guesses
           (setf numGuesses (+ 1 numGuesses))

           ; if this is the first guess then tell them what their possible score is
           (if (= 1 numGuesses)
               (format t "You are playing for a possible score of ~A~%~%" score))

           ; get the user's guess (position) and look up the char
           (setf guessPos (if (null (cdr mixedList)) 0 ; (here only one possible choice)
                              (getGuessPos sofarWord mixedList)))
           (setf guessChar (elt mixedList guessPos))

           ; print debugging info (during development)
           (debugPrt guessPos guessChar)

           ; check their guess
           (if (char= guessChar (car togoList))

               ; if correct, update sofarWord/List, togoWord/List, mixedWord/List
               (block 'goodGuess
                  (if (null (cdr togoList))
                      (format t "   You solved it!~%")
                      (format t "   Correct!~%"))
                  (setf sofarList (append sofarList (list guessChar)))
                  (setf sofarWord (format nil "~{~A~}" sofarList))
                  (setf togoList (cdr togoList))
                  (setf togoWord (format nil "~{~A~}" togoList))
                  (setf mixedList (remove guessChar mixedList :count 1))
                  (setf mixedWord (format nil "~{~A~}" mixedList))
                  )

               ; otherwise cut their score in half
               (block 'badGuess
                  (setf score (if (= score 1) 0 (/ score 2)))
                  (format t "   Incorrect!  Possible score is reduced to ~A~%" score)
                  ))

           ; print debugging info (during development)
           (if debugMode (debugPrt guessPos guessChar))
           )

   ))))


; (main [options])
; ----------------
; main routine, allows player to play multiple rounds
;    and tracks score information
(defun main (&optional (args nil))
   (let
      ((score 0) (total 0) (best 0) (numGames 0))
   (labels
      ((updatePrt ()
          (format t "Your score for round ~A was ~A, for a total of ~A.~%"
              numGames score total)
          (format t "   Your best round so far is ~A, with an average of ~A~%~%"
              best (if (> numGames 0) (/ total numGames) 0))))

   ; initialize the random number generator
   (setf *random-state* (make-random-state t))

   ; give the user an intro to the game
   (welcome)

   ; keep playing until they decide to quit
   (do  () ; no locals

        ( ; quit if they don't want to play again
          (not (playAgain numGames))
          (format t "~%Bye!~%~%"))

        (block 'playOneRound
        (setf numGames (+ numGames 1))
        (setf score (playRound numGames))
        (setf total (+ total score))
        (if (> score best) (setf best score))
        (updatePrt))))))


; start the game running
(main (cdr si::*command-args*))

'+
