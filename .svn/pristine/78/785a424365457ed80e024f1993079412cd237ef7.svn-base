package Function;
use warnings;
use strict;
use Carp;

#contructor
sub new {
    my ($classe) = @_;
    $classe = ref($classe) || $classe;
    my $this = {};
    bless($this, $classe);

    $this->{RET_TYPE}="null";
    $this->{FNAME}="null";
    $this->{ARGS}=[];
    $this->{NB_ARGS}=0;
    $this->{ALREADY_CALLED}=0;
    $this->{TYPE}="LIBRARY";

    $this->{ENTRY_CODE}= 0;
    $this->{EXIT_CODE}= 0;

    $this->{BODY_BEGIN}="";
    $this->{BODY_FCALL}="";
    $this->{BODY_END}="";
    $this->{USE_VARGS}=0;
    return $this;
}

sub add_body {
    my ($this, $new_line) = @_;

    if($this->{ALREADY_CALLED} == 0) {
	$this->{BODY_BEGIN}.="\t${new_line}";
    } else {
	$this->{BODY_END}.="\t${new_line}";
    }
}

sub add_event {
    my ($this, $code) = @_;
    my $nb_param = get_arg_num($this);

    if($this->{ALREADY_CALLED} == 0) {
	$this->{ENTRY_CODE}= $code;
    } else {
	$this->{EXIT_CODE}= $code;
    }
    $nb_param-- if ( $this->{USE_VARGS} );
    if($nb_param > 4) {
	$nb_param = 4;
    }
    my $string = "";
    if ($this->{TYPE} eq "LIBRARY") {
	$string .= "EZTRACE_EVENT${nb_param} ($code";
	my $i;
	for($i=0; $i<$nb_param; $i++) {
	    $string .= ", ";
	    $string .= get_arg_name($this, $i);
	}
	$string .= ");\n";
    }
    add_body($this, $string);
}

sub create_intercept {
    my ($this) = @_;
    my $ret = "";
    if ($this->{TYPE} eq "LIBRARY") {
	$ret = "INTERCEPT2(\"".$this->{FNAME}."\", lib".$this->{FNAME}.")\n";
    }
    return $ret;
}

# Create a string that has the following pattern:
# asprintf(&varname, "arg1=%x, arg2=%x", arg1, arg2);
# return the generated string
sub create_asprintf {
    my ($this, $output_var_name) = @_;
    my $format = "";
    my $args = "";

    my $i;
    my $nb_param = $this->get_arg_num;
    # construct the format and args at the same time
    for($i=0; $i<$nb_param; $i++) {
	my $var_name = $this->get_arg_name($i);
	my $index = $i + 1;
	$args .= ", GET_PARAM(CUR_EV, ".$index.")";

	$format .= "$var_name = %x";
	my $num_remaining_args=$nb_param - $i;
	if($num_remaining_args>1) {
	    $format .= ", ";
	}
    }

    my $ret = "asprintf(&${output_var_name}, \"${format}\" ${args} );";
    return $ret;
}

sub add_fcall {
    my ($this) = @_;
    if($this->{ALREADY_CALLED} != 0) {
	print "Error: calling twice function ".$this->{FNAME}."\n";
	exit 1;
    }
    $this->{ALREADY_CALLED} = 1;

    if ($this->{TYPE} eq "LIBRARY") {
	if ($this->{RET_TYPE} ne "void"){
	    $this->{BODY_FCALL} .= "\t".$this->{RET_TYPE}." ret = lib".$this->{FNAME}." (";
	} else {
	    $this->{BODY_FCALL} .= "\tlib".$this->{FNAME}." (";
	}
	my $i;
	my $array_size = get_arg_num($this);
	for($i=0; $i<$array_size; $i++) {
	    $this->{BODY_FCALL} .= get_arg_name($this, $i);

	    my $num_remaining_args=$array_size - $i;
	    if($num_remaining_args>1) {
		$this->{BODY_FCALL} .= ", ";
	    }
	}
	$this->{BODY_FCALL} .= ");\n";
    }
}

# Create the callback string (type (*libfname)(arg1, arg2...))
# returns the string to insert
sub create_callback {
    my ($this) = @_;
# create the '<type> (*lib<fname>) (' string
    my $ret = $this->{RET_TYPE}." (*lib".$this->{FNAME}.") (";

    my $i;
    my $nb_arg = get_arg_num($this);
# add the parameters to the string
    for($i=0; $i<$nb_arg; $i++) {
	$ret .= get_arg_type($this, $i)." ".get_arg_name($this, $i);
	my $num_remaining_args=$nb_arg - $i;
	if($num_remaining_args>1) {
	    $ret .=", ";
	}
    }
# finalize the string
    $ret .= ");\n";
    return $ret;
}

# Create the function string (type fname(arg1, arg2...))
# returns the string to insert
sub create_function {
    my ($this) = @_;
# create the '<type> <fname> (' string
    my $ret =  $this->{RET_TYPE}." ".$this->{FNAME}." (";

    my $i;
    my $nb_arg = get_arg_num($this);
# add the parameters to the string
    for($i=0; $i<$nb_arg; $i++) {
	$ret .= get_arg_type($this, $i)." ".get_arg_name($this, $i);
	my $num_remaining_args=$nb_arg - $i;
	if($num_remaining_args>1) {
	    $ret .=", ";
	}
    }
# finalize the string
    $ret .= ") {\n";

    # Add block for va_arg parameters
    if ( $this->{USE_VARGS} ) {
        $ret .= "\tva_list myargs;\n";
        $ret .= "\tva_start(myargs, ".$this->get_arg_name($nb_arg-2).");\n";
    }

    $ret .= $this->{BODY_BEGIN};
    if($this->{ALREADY_CALLED} == 0) {
	$this->add_fcall();
    }
    $ret .= $this->{BODY_FCALL};
    $ret .= $this->{BODY_END};
    if ($this->{RET_TYPE} ne "void"){
	$ret .="\treturn ret;\n";
    }
    $ret .="}\n";

    return $ret;
}

# return the number of parameters for the function
sub get_arg_num {
    my ($this) = @_;
    return $this->{NB_ARGS};
}

# return the ith parameter type
# params
#     $arg_num i
sub get_arg_type {
    my ($this, $arg_num) = @_;
    my @args = @{$this->{ARGS}};
    my @ith_arg = @{$args[$arg_num]};
    return $ith_arg[0];
}

# return the ith parameter name
# params
#     $arg_num i
sub get_arg_name {
    my ($this, $arg_num) = @_;
    my @args = @{$this->{ARGS}};
    my @ith_arg = @{$args[$arg_num]};
    return $ith_arg[1];
}

sub get_fname {
    my ($this) = @_;
    return $this->{FNAME};
}

# print information on the function for debugging
# params
#     $arg_num i
sub print {
    my ($this) = @_;
    printf "\tret type = ".$this->{RET_TYPE}."\n";
    printf "\tfname = ".$this->{FNAME}."\n";

    my $i;
    for($i=0; $i<$this->get_arg_num(); $i++) {
	print "type '".$this->get_arg_type($i)."'\t";
	print "name '".$this->get_arg_name($i)."'\n";
    }
}

# change the type returned by the function
# params
#     $ret_type return type
sub set_ret_type {
    my ($this, $ret_type) = @_;
    $this->{RET_TYPE} = $ret_type;
}

# change the function name
# params
#     $fname function name
sub set_fname( $$ ) {
    my ($this, $fname) = @_;
    $this->{FNAME} = $fname;
}

# change the function name
# params
#     $fname function name
sub set_type( $$ ) {
    my ($this, $type) = @_;
    $this->{TYPE} = $type;
}

# add a parameter to the function
# params
#     $new_arg_type parameter type
#     $new_arg_name parameter name
sub add_arg( $$$ ) {
    my ($this, $new_arg_type, $new_arg_name) = @_;
    my @new_arg = [$new_arg_type, $new_arg_name];
    push ((@{$this->{ARGS}}), @new_arg);
    $this->{NB_ARGS}++;
    $this->{USE_VARGS} = 1 if ($new_arg_name eq "..." );
}

# initialize the current library
sub lib_init {
    my ($libname, @func_tab) = @_;
    my $ret="START_INTERCEPT\n";
    foreach (@func_tab){
	   $ret .= "\t".create_intercept($_);
    }
    $ret .="END_INTERCEPT\n";
    $ret .= "static void __".$libname."_init (void) __attribute__ ((constructor));\n";
    $ret .= "static void __".$libname."_init (void) {\n";



    $ret .= "#ifdef EZTRACE_AUTOSTART\n";
    $ret .= "\teztrace_start ();\n";
    $ret .= "#endif\n";
    $ret .= "}\n";

    return $ret;
}

sub lib_conclude {
    my ($libname) = @_;
    my $ret = "static void __".$libname."_conclude (void) __attribute__ ((destructor));\n";
    $ret .= "static void __".$libname."_conclude (void){\n";
    $ret .= "\teztrace_stop ();\n";
    $ret .="}\n";

    return $ret;
}


# end of the module
1;
__END__
