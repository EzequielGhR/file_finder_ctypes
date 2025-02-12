import os


from argparse import ArgumentParser
from concurrent.futures import ThreadPoolExecutor
from wrapper import find_by_content, find_by_name


def find_combined(name: str, content: str, path: str):
    """
    Find combined results from name and/or contents on the given path.
    :param name: The name or partial name of the file you're searching for.
    :param content: Part of the contents of the file you're searching for.
    :param path: The path where to start searching for the file or files.
    :return: Dictionary with combined results, intersection of results, results by name, and results by contents.
    """
    # We need the path and some of the arguments, name or content
    if not path or not (content or name):
        return {}
    
    # Turn the path absolute and make sure it exists.
    path = os.path.abspath(path)
    if not os.path.exists(path):
        print("Path does not exist")
        return {}
    
    # Base level search, without recursive checking
    results_by_name = find_by_name(name, path, False) if name else {}
    results_by_content = find_by_content(content, path, False) if content else {}

    # initialize futures lists
    futures = {"by_name": [], "by_content": []}

    # Multithread searching on sub directories
    with ThreadPoolExecutor(10, thread_name_prefix="file_finder_ctypes") as executor:
        for entry in os.listdir(path):
            sub_path = f"{path}/{entry}"

            # Ignore non-directories
            if not os.path.isdir(sub_path):
                continue

            futures["by_name"].append(executor.submit(find_by_name, name, sub_path, True))
            futures["by_content"].append(executor.submit(find_by_content, content, sub_path, True))
    
    # Extend each list by future results
    for future in futures["by_name"]:
        results_by_name.extend(future.result())
    
    for future in futures["by_content"]:
        results_by_content.extend(future.result())
    
    # Initialize combined results, make a copy of each element to avoid modifying results_by_name by reference
    combined_result = {r["file_path"]: r.copy() for r in results_by_name}
    # Initialize the intersection of results
    intersection_result = [r for r in results_by_content if r["file_path"] in combined_result]

    # Update or add result to combined results.
    for result in results_by_content:
        if (file_path := result["file_path"]) in combined_result:
            combined_result[file_path].update(result)
        else:
            combined_result[file_path] = result
    
    # Turn into a list of records
    combined_result = list(combined_result.values())

    return {
        "combined_result": combined_result,
        "intersection_result": intersection_result,
        "results_only_by_name": results_by_name,
        "results_only_by_content": results_by_content
    }


def truncate_str(string: str, size: int):
    if len(string) < size:
        return string.ljust(size)
    return f"{string[:size-4]}...".ljust(size)


def make_clickable_path(path: str, size: int):
    """Wraps a file path with an ANSI hyperlink for supported terminals."""
    abs_path = os.path.abspath(path)
    return f"\033]8;;file://{abs_path}\033\\{truncate_str(abs_path, size)}\033]8;;\033\\"


def records_to_markdown_table(data: list[dict[str, str]]):
    if not data:
        return ""
    
    headers = [k for k in data[0]]
    headers_with_size = {
        header: min(max([len(record[header]) for record in data] + [len(header)]), 75)
        for header in headers
    }

    # Header format:    | key1    | key2      | key3 ...
    table_header = f'| {" | ".join([header.ljust(size) for header, size in headers_with_size.items()])} |'

    # Separator format: |---------|-----------|------ ...
    separator = f'| {" | ".join(["-" * size for size in headers_with_size.values()])} |'

    # Row format:       | value1  | value2    | value3 ...
    table_rows = "\n".join([
        # Row start
        f'| {
            " | ".join([make_clickable_path(record[header], size) if header == "file_path"
                            else truncate_str(record[header], size)
                        for header, size in headers_with_size.items()])
        # Row end
        } |' for record in data
    ])

    return "\n".join([table_header, separator, table_rows])


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--name", type=str)
    parser.add_argument("--content", type=str)
    parser.add_argument("--path", type=str)

    args = parser.parse_args()
    to_be_displayed = find_combined(args.name, args.content, args.path)
    
    if not to_be_displayed:
        print("No results found for parameters provided")
    
    combined_results = to_be_displayed.get("combined_result")
    intersection_results = to_be_displayed.get("intersection_result")
    results_by_name = to_be_displayed.get("results_only_by_name")
    results_by_content = to_be_displayed.get("results_only_by_content")

    print("*" * 100)

    if results_by_name:
        print("Results found by name:")
        print(records_to_markdown_table(results_by_name))
        print()
    
    if results_by_content:
        print("Results found by content:")
        print(records_to_markdown_table(results_by_content))
        print()

    if combined_results:
        print("Combined results:")
        print(records_to_markdown_table(combined_results))
        print()

    if intersection_results:
        print("Results intersection:")
        print(records_to_markdown_table(intersection_results))
        print()
