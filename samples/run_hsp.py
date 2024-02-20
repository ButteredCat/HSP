#!/usr/bin/env python3
from multiprocessing import Pool
import json
import subprocess
import timeit


class Order(object): 
  def __init__(self, i):
    self.i = i
    self.input = {
    "filename": "/home/xiaoyc/dataset/hsp_unittest/speed/test813_640-640-120.tif",
    "raw": False
    }
    self.coeff = {
      "dark_a": "/home/xiaoyc/dataset/hsp_unittest/speed/VNIR/dark_a.tif",
      "dark_b": "/home/xiaoyc/dataset/hsp_unittest/speed/VNIR/dark_b.tif",
      "badpixel": "/home/xiaoyc/dataset/hsp_unittest/speed/VNIR/badpixel.tif",
      "rel_a": "/home/xiaoyc/dataset/hsp_unittest/speed/VNIR/rel_a.tif",
      "rel_b": "/home/xiaoyc/dataset/hsp_unittest/speed/VNIR/rel_b.tif",
      "etalon_a": "/home/xiaoyc/dataset/hsp_unittest/speed/VNIR/etalon_a.tif",
      "etalon_b": "/home/xiaoyc/dataset/hsp_unittest/speed/VNIR/etalon_b.tif"
    }
  
  def write(self, filename):
    output = "/tmp/hsp/out_%d.tif" % self.i
    json_order = {
      "input": [self.input],
      "coeff": self.coeff,
      "output": [output]
    }
    with open(filename, "w") as f:
      json.dump(json_order, f, indent=2)
  

def run(order):
  subprocess.call(["/home/xiaoyc/repo/hsp/bin/hsp", order])


if __name__ == '__main__':
  n = 10
  order_lst = []
  for i in range(n):
    order = Order(i)
    order_name = "/tmp/hsp/order_%d.json" % i
    order.write(order_name)
    order_lst.append(order_name)
  
  def run_parallel():
    with Pool(n) as p:
      p.map(run, order_lst)
    
  print(timeit.timeit(stmt=run_parallel, number=1))