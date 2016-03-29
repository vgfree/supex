

local old = "\n<<<<<<< HEAD\nlualog.open('access')\nlualog.open('manage')\n=======\nlualog.open('manage')\nlualog.open('access')\n>>>>>>> origin/devel\n\n\n\n<<<<<<< HEAD\nlualog.open('access')\nlualog.open('manage')\n=======\nlualog.open('manage')\nlualog.open('access')\n>>>>>>> origin/devel\n"

local new = string.gsub(old, '\n<<<<<<< HEAD\n.-=======\n(.-)>>>>>>> origin/devel\n', "\n%1\n")


print(new)
