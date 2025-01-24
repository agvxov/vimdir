require 'etc'

# NOTE:
#  Every invocation which doesnt *have to* be a moist run should be a dry run.
#  Tests require `./vimdir_test_file.vimdir`,
#   therefor they will only succeed on debug builds.

#  ___          _
# | _ ) __ _ __(_)__
# | _ \/ _` (_-< / _|
# |___/\__,_/__/_\__|
class CMDTEST_basic < Cmdtest::Testcase
  def test_help
    cmd "vimdir -h" do
      exit_zero
      stdout_equal /.+/
    end
  end

  def test_no_color
    cmd "NO_COLOR=1 vimdir -h" do
      exit_zero
      stdout_equal /^[^\e]+$/m
    end
  end

  def test_wrong_arg
    cmd "vimdir -j" do
      exit_nonzero
      stdout_equal /.+/
      stderr_equal /.+/
    end
  end

  def test_missing_folder
    cmd "vimdir -n ./this/directory/does/not/exist/" do
      exit_nonzero
      created_files ["vimdir_test_file.vimdir"]
      stderr_equal /\A.*error.+\n.*notice.+\n\z/
    end
  end

  def test_editor
    import_file "test/saver.sh", "./"

    cmd "EDITOR=./saver.sh vimdir -n ./" do
      exit_zero
      created_files ["output.txt"]
    end
  end

  def test_vimdir_editor
    import_file "test/saver.sh", "./"

    cmd "VIMDIREDITOR=./saver.sh vimdir -n ./" do
      exit_zero
      created_files ["output.txt"]
    end
  end
end



#  __  __         _ _
# |  \/  |_  _ __| (_)_ _
# | |\/| | || / _` | | '_|
# |_|  |_|\_, \__,_|_|_|
#         |__/
class CMDTEST_mydir < Cmdtest::Testcase
  def setup
    import_file "test/replacer.sh",   "./"
    import_file "test/saver.sh",      "./"
    import_file "test/memoryhole.sh", "./"
    import_file "test/trash.sh",      "./"
    import_directory "test/mydir/", "./mydir/"
  end

  def test_noop
    cmd "EDITOR=./memoryhole.sh vimdir -n ./mydir/" do
      exit_zero
    end
  end

  def test_swapped_order_noop
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "002\t./mydir/script.sh",
        "001\t./mydir/file.txt",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./mydir/" do
      exit_zero
      removed_files ["target.txt"]
    end
  end

  def test_contents
    expected = [
        "000\t./mydir/.gitkeep",
        "001\t./mydir/file.txt",
        "002\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n ./mydir/" do
      exit_zero
      created_files ["output.txt"]
      file_equal "output.txt", expected
    end
  end

  def test_false_entry
    File.write('target.txt',
      [
        "005\t./mydir/.gitkeep",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./mydir/" do
      exit_nonzero
      created_files ["vimdir_test_file.vimdir"]
      removed_files ["target.txt"]
      stderr_equal /\A.*error.+\n.*notice.+\n\z/
    end
  end

  def test_permission_contents
    expected = [
        "000\t-rw-r--r--\t./mydir/.gitkeep",
        "001\t-rw-r--r--\t./mydir/file.txt",
        "002\t-rwxr-xr-x\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -p ./mydir/" do
      exit_zero
      created_files ["output.txt"]
      file_equal "output.txt", expected
    end
  end

  def test_owner_contents
    username  = Etc.getpwuid(1000).name
    groupname = Etc.getgrgid(1000).name
    expected = [
        "000\t#{username}:#{groupname}\t./mydir/.gitkeep",
        "001\t#{username}:#{groupname}\t./mydir/file.txt",
        "002\t#{username}:#{groupname}\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -o ./mydir/" do
      exit_zero
      created_files ["output.txt"]
      file_equal "output.txt", expected
    end
  end

  def test_permission_owner_contents
    username  = Etc.getpwuid(1000).name
    groupname = Etc.getgrgid(1000).name
    expected = [
        "000\t-rw-r--r--\t#{username}:#{groupname}\t./mydir/.gitkeep",
        "001\t-rw-r--r--\t#{username}:#{groupname}\t./mydir/file.txt",
        "002\t-rwxr-xr-x\t#{username}:#{groupname}\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -p -o ./mydir/" do
      exit_zero
      created_files ["output.txt"]
      file_equal "output.txt", expected
    end
  end

  def test_dry_del
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "002\t./mydir/script.sh"
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./mydir/" do
      exit_zero
      removed_files ["target.txt"]
      stderr_equal /\A.*delete '.*file.txt'.*\n\z/
    end
  end

  def test_dry_chmod_file
    File.write('target.txt',
      [
        "000\t-rw-r--r--\t./mydir/.gitkeep",
        "001\t-rw-r--r--\t./mydir/file.txt",
        "002\t-rw-r--r--\t./mydir/script.sh",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n -p ./mydir/" do
      exit_zero
      removed_files ["target.txt"]
      stderr_equal /\A.*chmod '.*script.sh' \(.+\).*\n\z/
    end
  end

  def test_dry_copy_file
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "001\t./mydir/file.txt",
        "001\t./mydir/file2.txt",
        "002\t./mydir/script.sh",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./mydir/" do
      exit_zero
      removed_files ["target.txt"]
      stderr_equal /\A.*copy.*'.*file2.txt'.*\n\z/
    end
  end

  def test_dry_touch_file
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "001\t./mydir/file.txt",
        "002\t./mydir/script.sh",
        "./mydir/new.txt",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./mydir/" do
      exit_nonzero
      created_files ["vimdir_test_file.vimdir"]
      removed_files ["target.txt"]
      stderr_equal /\A.*touch '.*new.txt'.*\n.*error.+\n.*notice.+\n\z/
    end
  end

  def test_del
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "002\t./mydir/script.sh",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir ./mydir/" do
      exit_zero
      removed_files ["target.txt", "mydir/file.txt"]
    end
  end

  def test_del_custom
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "002\t./mydir/script.sh",
      ].join("\n")
    )

    cmd "VIMDIRRM=./trash.sh EDITOR=./replacer.sh vimdir ./mydir/" do
      exit_zero
      created_files ["mydir/file.txt.trash"]
      removed_files ["target.txt", "mydir/file.txt"]
    end
  end

  def test_copy_file
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "001\t./mydir/file.txt",
        "001\t./mydir/file2.txt",
        "002\t./mydir/script.sh",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir ./mydir/" do
      exit_zero
      created_files ["mydir/file2.txt"]
      removed_files ["target.txt"]
    end
  end

  def test_touch_file
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "001\t./mydir/file.txt",
        "002\t./mydir/script.sh",
        "./mydir/new.txt",
      ].join("\n")
    cmd "EDITOR=./replacer.sh vimdir ./mydir/" do
      exit_zero
      created_files ["mydir/new.txt"]
      removed_files ["target.txt"]
end



#  _  _        _          _    _ _
# | \| |___ __| |_ ___ __| |__| (_)_ _
# | .` / -_|_-<  _/ -_) _` / _` | | '_|
# |_|\_\___/__/\__\___\__,_\__,_|_|_|
class CMDTEST_mynesteddir < Cmdtest::Testcase
  def setup
    import_file "test/replacer.sh",   "./"
    import_file "test/saver.sh",      "./"
    import_directory "test/mynesteddir/", "./mynesteddir/"
  end

  def test_trailing_slash_included_contents
    cmd "EDITOR=./saver.sh vimdir -n ./mynesteddir/" do
      exit_zero
      created_files ["output.txt"]
      file_equal "output.txt", /^000\t.+nest\/$/
    end
  end

  def test_recursive_contents
    expected = [
        "000\t./mynesteddir/nest/",
        "001\t./mynesteddir/nest/.gitkeep",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -r ./mynesteddir/" do
      exit_zero
      created_files ["output.txt"]
      file_equal "output.txt", expected
    end
  end
end



#  ___                      _ _
# / __|_ __ ____ _ _ __  __| (_)_ _
# \__ \ V  V / _` | '_ \/ _` | | '_|
# |___/\_/\_/\__,_| .__/\__,_|_|_|
#                 |_|
class CMDTEST_myswapdir < Cmdtest::Testcase
  def setup
    import_file "test/replacer.sh", "./"
    import_directory "test/myswapdir/", "./myswapdir/"
  end

  def test_dry_swap
    File.write('target.txt',
      [
        "000\t./myswapdir/file2.txt",
        "001\t./myswapdir/file1.txt",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./myswapdir/" do
      exit_zero
      removed_files ["target.txt"]
      stderr_equal /.+swap.+/
    end
  end

  def test_swap
    File.write('target.txt',
      [
        "000\t./myswapdir/file2.txt",
        "001\t./myswapdir/file1.txt",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir ./myswapdir/" do
      exit_zero
      removed_files ["target.txt"]
      changed_files ["myswapdir/file1.txt", "myswapdir/file2.txt"]
    end
  end
end
