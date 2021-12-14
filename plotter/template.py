from util_types import *

color_list = {
    'white':              'ffffff', #ffffff = 255 255 255
    'black':              '000000', #000000 =   0   0   0
    'dark-grey':          'a0a0a0', #a0a0a0 = 160 160 160
    'red':                'ff0000', #ff0000 = 255   0   0
    'web-green':          '00c000', #00c000 =   0 192   0
    'web-blue':           '0080ff', #0080ff =   0 128 255
    'dark-magenta':       'c000ff', #c000ff = 192   0 255
    'dark-cyan':          '00eeee', #00eeee =   0 238 238
    'dark-orange':        'c04000', #c04000 = 192  64   0
    'dark-yellow':        'c8c800', #c8c800 = 200 200   0
    'royalblue':          '4169e1', #4169e1 =  65 105 225
    'goldenrod':          'ffc020', #ffc020 = 255 192  32
    'dark-spring-green':  '008040', #008040 =   0 128  64
    'purple':             'c080ff', #c080ff = 192 128 255
    'steelblue':          '306080', #306080 =  48  96 128
    'dark-red':           '8b0000', #8b0000 = 139   0   0
    'dark-chartreuse':    '408000', #408000 =  64 128   0
    'orchid':             'ff80ff', #ff80ff = 255 128 255
    'aquamarine':         '7fffd4', #7fffd4 = 127 255 212
    'brown':              'a52a2a', #a52a2a = 165  42  42
    'yellow':             'ffff00', #ffff00 = 255 255   0
    'turquoise':          '40e0d0', #40e0d0 =  64 224 208
    'grey0':              '000000', #000000 =   0   0   0
    'grey10':             '1a1a1a', #1a1a1a =  26  26  26
    'grey20':             '333333', #333333 =  51  51  51
    'grey30':             '4d4d4d', #4d4d4d =  77  77  77
    'grey40':             '666666', #666666 = 102 102 102
    'grey50':             '7f7f7f', #7f7f7f = 127 127 127
    'grey60':             '999999', #999999 = 153 153 153
    'grey70':             'b3b3b3', #b3b3b3 = 179 179 179
    'grey':               'c0c0c0', #c0c0c0 = 192 192 192
    'grey80':             'cccccc', #cccccc = 204 204 204
    'grey90':             'e5e5e5', #e5e5e5 = 229 229 229
    'grey100':            'ffffff', #ffffff = 255 255 255
    'light-red':          'f03232', #f03232 = 240  50  50
    'light-green':        '90ee90', #90ee90 = 144 238 144
    'light-blue':         'add8e6', #add8e6 = 173 216 230
    'light-magenta':      'f055f0', #f055f0 = 240  85 240
    'light-cyan':         'e0ffff', #e0ffff = 224 255 255
    'light-goldenrod':    'eedd82', #eedd82 = 238 221 130
    'light-pink':         'ffb6c1', #ffb6c1 = 255 182 193
    'light-turquoise':    'afeeee', #afeeee = 175 238 238
    'gold':               'ffd700', #ffd700 = 255 215   0
    'green':              '00ff00', #00ff00 =   0 255   0
    'dark-green':         '006400', #006400 =   0 100   0
    'spring-green':       '00ff7f', #00ff7f =   0 255 127
    'forest-green':       '228b22', #228b22 =  34 139  34
    'sea-green':          '2e8b57', #2e8b57 =  46 139  87
    'blue':               '0000ff', #0000ff =   0   0 255
    'dark-blue':          '00008b', #00008b =   0   0 139
    'midnight-blue':      '191970', #191970 =  25  25 112
    'navy':               '000080', #000080 =   0   0 128
    'medium-blue':        '0000cd', #0000cd =   0   0 205
    'skyblue':            '87ceeb', #87ceeb = 135 206 235
    'cyan':               '00ffff', #00ffff =   0 255 255
    'magenta':            'ff00ff', #ff00ff = 255   0 255
    'dark-turquoise':     '00ced1', #00ced1 =   0 206 209
    'dark-pink':          'ff1493', #ff1493 = 255  20 147
    'coral':              'ff7f50', #ff7f50 = 255 127  80
    'light-coral':        'f08080', #f08080 = 240 128 128
    'orange-red':         'ff4500', #ff4500 = 255  69   0
    'salmon':             'fa8072', #fa8072 = 250 128 114
    'dark-salmon':        'e9967a', #e9967a = 233 150 122
    'khaki':              'f0e68c', #f0e68c = 240 230 140
    'dark-khaki':         'bdb76b', #bdb76b = 189 183 107
    'dark-goldenrod':     'b8860b', #b8860b = 184 134  11
    'beige':              'f5f5dc', #f5f5dc = 245 245 220
    'olive':              'a08020', #a08020 = 160 128  32
    'orange':             'ffa500', #ffa500 = 255 165   0
    'violet':             'ee82ee', #ee82ee = 238 130 238
    'dark-violet':        '9400d3', #9400d3 = 148   0 211
    'plum':               'dda0dd', #dda0dd = 221 160 221
    'dark-plum':          '905040', #905040 = 144  80  64
    'dark-olivegreen':    '556b2f', #556b2f =  85 107  47
    'orangered4':         '801400', #801400 = 128  20   0
    'brown4':             '801414', #801414 = 128  20  20
    'sienna4':            '804014', #804014 = 128  64  20
    'orchid4':            '804080', #804080 = 128  64 128
    'mediumpurple3':      '8060c0', #8060c0 = 128  96 192
    'slateblue1':         '8060ff', #8060ff = 128  96 255
    'yellow4':            '808000', #808000 = 128 128   0
    'sienna1':            'ff8040', #ff8040 = 255 128  64
    'tan1':               'ffa040', #ffa040 = 255 160  64
    'sandybrown':         'ffa060', #ffa060 = 255 160  96
    'light-salmon':       'ffa070', #ffa070 = 255 160 112
    'pink':               'ffc0c0', #ffc0c0 = 255 192 192
    'khaki1':             'ffff80', #ffff80 = 255 255 128
    'lemonchiffon':       'ffffc0', #ffffc0 = 255 255 192
    'bisque':             'cdb79e', #cdb79e = 205 183 158
    'honeydew':           'f0fff0', #f0fff0 = 240 255 240
    'slategrey':          'a0b6cd', #a0b6cd = 160 182 205
    'seagreen':           'c1ffc1', #c1ffc1 = 193 255 193
    'antiquewhite':       'cdc0b0', #cdc0b0 = 205 192 176
    'chartreuse':         '7cff40', #7cff40 = 124 255  64
    'greenyellow':        'a0ff20', #a0ff20 = 160 255  32
    'gray':               'bebebe', #bebebe = 190 190 190
    'light-gray':         'd3d3d3', #d3d3d3 = 211 211 211
    'light-grey':         'd3d3d3', #d3d3d3 = 211 211 211
    'dark-gray':          'a0a0a0', #a0a0a0 = 160 160 160
    'slategray':          'a0b6cd', #a0b6cd = 160 182 205
    'gray0':              '000000', #000000 =   0   0   0
    'gray10':             '1a1a1a', #1a1a1a =  26  26  26
    'gray20':             '333333', #333333 =  51  51  51
    'gray30':             '4d4d4d', #4d4d4d =  77  77  77
    'gray40':             '666666', #666666 = 102 102 102
    'gray50':             '7f7f7f', #7f7f7f = 127 127 127
    'gray60':             '999999', #999999 = 153 153 153
    'gray70':             'b3b3b3', #b3b3b3 = 179 179 179
    'gray80':             'cccccc', #cccccc = 204 204 204
    'gray90':             'e5e5e5', #e5e5e5 = 229 229 229
    'gray100':            'ffffff', #ffffff = 255 255 255
}

pointtypes = {
        'square':               4,
        'circle':               6,
        'up_triangle':          8,
        'down_triangle':        10,
        'filled_square':        5,
        'filled_circle':        7,
        'filled_up_triangle':   9,
        'filled_down_triangle': 11,
}

dashtypes = {
        'full':      1,
        'dashed':    2,
        'dotted':    3,
        'dot-dash':  4,
        'ddot-dash': 5,
}

chosen_colors = [ 'dark-red', 'blue', 'forest-green', 'navy', 'pink', 'grey', 'orange']

def print_header(f, name,\
        dim = (1800, 1200),\
        font = ('Verdana', 30),\
        linewidth = 5
        ):
    template_header = \
'''
reset

# png
set terminal pngcairo size {w},{h} enhanced font '{fname},{fsize}'
set output '{png_out_file}'

set border linewidth {linew}
set key outside above
'''
    print(template_header.format( w = dim[0], h = dim[1], fname = font[0], fsize = font[1], png_out_file = name, linew = linewidth), file = f)

def print_linetype(f, line_n, line_color,\
        linewidth = 5,
        pointsize = 3):
    pointtype = list(pointtypes.values())[(line_n - 1) % len(pointtypes)]
    dashtype  = dashtypes["dashed"]
    template_linetype =\
'''
# Set color of linestyle {id} to #{color}
set style line {id} linecolor rgb '#{color}' linetype {id} linewidth {lw} pointtype {pttype} pointsize {ptsize} dashtype {dtype}
# Set yerror color of linestyle {yid} to #{color}
set style line {yid} linecolor rgb '#{color}' linetype {id} linewidth {ylw} pointtype {pttype} pointsize {ptsize}
'''
    print(template_linetype.format(id = 2 * line_n, yid = 2 * line_n + 1, color = line_color, lw = linewidth, ylw = 0.75 * linewidth, pttype = pointtype, ptsize = pointsize, dtype = dashtype), file = f)

def print_axis(f,\
        tic_scale = 0.75, xlabel = None, ylabel = None,
        xrangex = None, yrangex = None):
    print("# set axis", file = f)
    print("set tics scale {}".format(tic_scale), file = f)
    if xlabel: print("set xlabel \'{}\'".format(xlabel), file = f)
    if ylabel: print("set ylabel \'{}\'".format(ylabel), file = f)
    print("set xrange [0:*]", file = f);
    print("set yrange [0:*]", file = f);


def plot_cmd_str(title, datafile, line_id, yerrorbars = True):
    if yerrorbars:
        if title:
            return '\'{dfile}\' title \'{t}\' with yerrorbars linestyle {yid}, \'\' notitle with lines linestyle {id}'\
                    .format(dfile = datafile, t = title, id = 2 * line_id, yid = 2 * line_id + 1)
        else:
            return '\'{dfile}\' title \'\' with yerrorbars linestyle {yid}, \'\' notitle with lines linestyle {id}'\
                    .format(dfile = datafile, id = 2 * line_id, yid = 2 * line_id + 1)
    else:
        if title:
            return '\'{dfile}\' title \'{t}\' with lines linestyle {id}'\
                    .format(dfile = datafile, t = title, id = 2 * line_id, yid = 2 * line_id + 1)
        else:
            return '\'{dfile}\' title \'\' with lines linestyle {id}'\
                    .format(dfile = datafile, id = 2 * line_id, yid = 2 * line_id + 1)


def print_plot(f, arglist):
    print('plot ' + ', '.join(plot_cmd_str(t, dfile, id, yerror)\
            for t, dfile, id, yerror in arglist), file = f)


