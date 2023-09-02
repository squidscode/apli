(let ((x 10))
    (defun f ()
        x)
    
    (let ((x 20))
        (write (f))
        (terpri)))