(module
 (func $good (param $x0 i32)
  (local $x1 i32)
  (local.set $x1
   (i32.add
    (local.get $x0)
    (i32.const 28)
   )
  )
  (drop
   (local.get $x1)
  )
  (drop
   (local.get $x1)
  )
 )
 (func $tricky (param $x0 i32)
  (local $x1 i32)
  (local.set $x1
   (i32.add
    (local.get $x0)
    (i32.const 28)
   )
  )
  (drop
   (local.get $x1)
  )
  (local.set $x1
   (i32.add
    (local.get $x0)
    (i32.const 32)
   )
  )
  (drop
   (local.get $x1)
  )
 )
 (func $bad (param $x0 i32)
  (local $x1 i32)
  (if (i32.const 0)
   (local.set $x1
    (i32.add
     (local.get $x0)
     (i32.const 28)
    )
   )
  )
  (drop
   (local.get $x1)
  )
  (drop
   (local.get $x1)
  )
 )
)

