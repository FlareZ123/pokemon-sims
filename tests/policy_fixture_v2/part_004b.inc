      std::cout << "PASS " << name << '\n';
    } catch (const std::exception& error) {
      ++failures;
      std::cerr << "FAIL " << name << ": " << error.what() << '\n';
    }
  }
  std::cout << "Policy fixture tests: " << (tests.size() - static_cast<std::size_t>(failures)) << '/' << tests.size() << " passed\n";
  return failures == 0 ? 0 : 1;
}
