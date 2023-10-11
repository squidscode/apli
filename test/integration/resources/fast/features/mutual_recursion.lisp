(defun parity (x)
    (if (= x 0)
        "EVEN"
        (parity-helper (- x 1))))

(defun parity-helper (x)
    (if (= x 0)
        "ODD"
        (parity (- x 1))))

(write-line (parity 1))
(write-line (parity 20))
(write-line (parity 30))
(write-line (parity 25))
(write-line (parity 13))
(write-line (parity 50))
(write-line (parity 59))