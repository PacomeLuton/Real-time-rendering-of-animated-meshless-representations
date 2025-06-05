import bpy
import numpy as np

print("------------------")

R = bpy.data.objects[0].matrix_world.to_3x3()
t = bpy.data.objects[0].matrix_world.translation

R = np.array(R)
t = np.array(t)

myfile = open("miammiam.save","w")

for bone in bpy.data.objects[0].data.bones:
    head = bone.head_local
    head = np.array(head)
    l_head = np.dot(R, head) + t

    tail = bone.tail_local
    tail = np.array(tail)
    l_tail = np.dot(R, tail) + t
    
    print(bone.name)
    print(f"head = {l_head}")
    print(f"tail = {l_tail}")

    myfile.write(bone.name)
    myfile.write(f" {l_head[0]} {l_head[1]} {l_head[2]} ")
    myfile.write(f"{l_tail[0]} {l_tail[1]} {l_tail[2]}\n")