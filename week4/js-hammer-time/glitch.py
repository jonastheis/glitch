from flask import Flask, request
from random import randint

import subprocess, json, struct, frida



# ------------------------------------------
# 				Constants

HTML_FILE = "glitch.html"
PAGEMAP_FILE = "maps.bin"
FRIDA_FILE = "flipper.js"
KGSL_FILE = "kgsl.txt"
PAGEMAP_ENTRY_SIZE = 8 # 64 bits
PAGE_SHIFT = 12 # 1<<12 == 4096
FLIP_CHANCE = 55 # probability of triggering bit flips [1,100]


# ------------------------------------------
# 				Globals

PID = 0;

session = None
script = None
frida_api = None

textures = None
hammered_textures = {}
hammered_arrays = {}
arr_pages = None;



# ------------------------------------------
# 				Functions


def read_pfn(v_addr):
	
	def extract_pfn(entry):
		return entry & 0x7FFFFFFFFFFFFF

	with open(PAGEMAP_FILE, "rb") as f:
		offset = (v_addr>>PAGE_SHIFT) * PAGEMAP_ENTRY_SIZE
		f.seek(offset, 0)
		return extract_pfn(struct.unpack('Q', f.read(PAGEMAP_ENTRY_SIZE))[0])



class Template():
	def __init__(self, tex, offset, bit_to_flip):	
		self.tex = tex
		self.offset = offset
		self.bit_to_flip = bit_to_flip


class Texture():

	def __init__(self, v_addr, tex_id, tex_size):
		self.v_addr = v_addr
		self.tex_id = tex_id
		self.tex_size = tex_size
		self.pfn = 0

	# @TODO implement this function
	def get_pfn(self):
		self.pfn = read_pfn(self.v_addr)

	def __repr__(self):
		return "[{:#08x}, {:#04d}, {:#04d}, {:#08x}]".format(self.v_addr, self.tex_id, self.tex_size, self.pfn)


def get_kgsl_infos():
	cmd = "adb shell cat /d/kgsl/proc/{0}/mem".format(PID)
	with open(KGSL_FILE, "wb") as f:
		f.write(subprocess.check_output(cmd.split()))

	with open(KGSL_FILE, "rb") as f:
		lines = f.readlines()
		lines = [x.strip(b"\r\n") for x in lines]
		lines = [x for x in lines if b"texture" in x and b"(null)" not in x]

	textures = []
	for l in lines:
		split = l.split()
		textures.append(Texture(int(split[0], 16), int(split[3]), int(split[2])))
		
	return textures


def read_pagemap():
	cmd = "adb exec-out su -c  cat /proc/{0}/pagemap".format(PID)
	with open(PAGEMAP_FILE, "wb") as f:
			f.write(subprocess.check_output(cmd.split()))






# ------------------------------------------
# 			Flask Web Server


app = Flask(__name__)

@app.route('/')
def start():
	global PID, session, script, frida_api
	
	cmd = "adb shell pgrep firefox"
	PID = int(subprocess.check_output(cmd.split()).split()[0])
	print("Firefox PID: " + str(PID)) 

	# session = frida.get_usb_device().attach(PID)
	# with open(FRIDA_FILE, "r") as f:
	# 	script = session.create_script(f.read())	
	# script.load();
	# frida_api = script.exports

	with open(HTML_FILE, "r") as f:
		text = f.read()
		return text


@app.route('/get_tex_infos')
def get_tex_infos():
	global textures
	textures = get_kgsl_infos()
	# for t in textures:
	# 	print(t)
	read_pagemap()
	[tex.get_pfn() for tex in textures]
	# pp.pprint(textures)
	return json.dumps([tex.__dict__ for tex in textures])

	


@app.route('/hammer_tex')
def hammer_tex():
	global textures
	# pp.pprint(textures)
	target = None
	tex_id = int(request.args.get('tex_id'));
	print("Texture to hammer:" + str(tex_id))
	for t in textures:
		if t.tex_id == tex_id:
			target = t
	
	should_flip = (randint(0,100) >= 100 - FLIP_CHANCE)

	if not should_flip:
		hammered_textures[tex_id] = Template(target, -1, -1)
		return json.dumps({'offset': -1, 'bit_flip': -1})


	bit_to_flip = randint(0,64-1);
	offset = ((randint(0, 4095))>>3)<<3
	hammered_textures[tex_id] = Template(target, offset, bit_to_flip)

	frida_api.flip_bit(target.v_addr+offset, bit_to_flip);
	return json.dumps({'offset': offset, 'bit_flip': bit_to_flip})




@app.route('/hammer_array')
def hammer_array():
	global hammered_textures, hammered_arrays, arr_pages
	target = None
	tex_id = int(request.args.get('tex_id'));
	template = hammered_textures[tex_id]
	if arr_pages == None:
		arr_pages = frida_api.find_pages();

	if tex_id not in hammered_arrays:
		hammer_target = arr_pages[randint(0, len(arr_pages)-1)]
		hammered_arrays[tex_id] = hammer_target;

	hammer_target = hammered_arrays[tex_id]
		

	page_addr = (int(hammer_target['address'], 16)>>12)<<12;
	hammer_addr = page_addr + template.offset;
	print("Page_addr {:#08x}, hammer_addr: {:#08x}".format(page_addr, hammer_addr))
	frida_api.flip_bit(hammer_addr, template.bit_to_flip);

	return "Hammered"


